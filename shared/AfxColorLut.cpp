#include "stdafx.h"

#include "AfxColorLut.h"

#include <shared/AfxMath.h>

#include <algorithm>


const char CAfxColorLut::m_Magic[17] = { 'A','f','x','R','b','a','L','o','o','k','u','p','T','r','e','e','\0' };

bool CAfxColorLut::Query(const CRgba& color, CRgba* outColor)
{
	if (nullptr == m_Root) return false;

	float xR0, xR1;
	GreenLookupTreeNode_t* yR0, * yR1;
	GetInterval<GreenLookupTreeNode_t>(m_Root, color.R, xR0, xR1, yR0, yR1);

	float xG00, xG01, xG10, xG11;
	BlueLookupTreeNode_t* yG00, * yG01, * yG10, * yG11;
	GetInterval<BlueLookupTreeNode_t>(yR0, color.G, xG00, xG01, yG00, yG01);
	GetInterval<BlueLookupTreeNode_t>(yR1, color.G, xG10, xG11, yG10, yG11);

	float xB000, xB001, xB010, xB011, xB100, xB101, xB110, xB111;
	AlphaLookupTreeNode_t* yB000, * yB001, * yB010, * yB011, * yB100, * yB101, * yB110, * yB111;
	GetInterval<AlphaLookupTreeNode_t>(yG00, color.B, xB000, xB001, yB000, yB001);
	GetInterval<AlphaLookupTreeNode_t>(yG01, color.B, xB010, xB011, yB010, yB011);
	GetInterval<AlphaLookupTreeNode_t>(yG10, color.B, xB100, xB101, yB100, yB101);
	GetInterval<AlphaLookupTreeNode_t>(yG11, color.B, xB110, xB111, yB110, yB111);

	float xA0000, xA0001, xA0010, xA0011, xA0100, xA0101, xA0110, xA0111, xA1000, xA1001, xA1010, xA1011, xA1100, xA1101, xA1110, xA1111;
	CRgba* yA0000, * yA0001, * yA0010, * yA0011, * yA0100, * yA0101, * yA0110, * yA0111, * yA1000, * yA1001, * yA1010, * yA1011, * yA1100, * yA1101, * yA1110, * yA1111;
	GetInterval<CRgba>(yB000, color.A, xA0000, xA0001, yA0000, yA0001);
	GetInterval<CRgba>(yB001, color.A, xA0010, xA0011, yA0010, yA0011);
	GetInterval<CRgba>(yB010, color.A, xA0100, xA0101, yA0100, yA0101);
	GetInterval<CRgba>(yB011, color.A, xA0110, xA0111, yA0110, yA0111);
	GetInterval<CRgba>(yB100, color.A, xA1000, xA1001, yA1000, yA1001);
	GetInterval<CRgba>(yB101, color.A, xA1010, xA1011, yA1010, yA1011);
	GetInterval<CRgba>(yB110, color.A, xA1100, xA1101, yA1100, yA1101);
	GetInterval<CRgba>(yB111, color.A, xA1110, xA1111, yA1110, yA1111);

	CRgba x[16] = {
		CRgba(xR0, xG00, xB000, xA0000),
		CRgba(xR0, xG00, xB000, xA0001),
		CRgba(xR0, xG00, xB001, xA0010),
		CRgba(xR0, xG00, xB001, xA0011),
		CRgba(xR0, xG01, xB010, xA0100),
		CRgba(xR0, xG01, xB010, xA0101),
		CRgba(xR0, xG01, xB011, xA0110),
		CRgba(xR0, xG01, xB011, xA0111),
		CRgba(xR1, xG10, xB100, xA1000),
		CRgba(xR1, xG10, xB100, xA1001),
		CRgba(xR1, xG10, xB101, xA1010),
		CRgba(xR1, xG10, xB101, xA1011),
		CRgba(xR1, xG11, xB110, xA1100),
		CRgba(xR1, xG11, xB110, xA1101),
		CRgba(xR1, xG11, xB111, xA1110),
		CRgba(xR1, xG11, xB111, xA1111),
	};

	float wSum = 0;
	float w[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (int i = 0; i < 16; ++i)
	{
		float cw = std::abs(x[i].R - color.R) * std::abs(x[i].G - color.G) * std::abs(x[i].B - color.B) * std::abs(x[i].A - color.A);
		wSum += cw;
		w[i] = cw;
	}

	if (0 == wSum) return false;

	if (outColor)
	{
		CRgba y[16] = {
			ValueOrDefault(yA0000, color),
			ValueOrDefault(yA0001, color),
			ValueOrDefault(yA0010, color),
			ValueOrDefault(yA0011, color),
			ValueOrDefault(yA0100, color),
			ValueOrDefault(yA0101, color),
			ValueOrDefault(yA0110, color),
			ValueOrDefault(yA0111, color),
			ValueOrDefault(yA1000, color),
			ValueOrDefault(yA1001, color),
			ValueOrDefault(yA1010, color),
			ValueOrDefault(yA1011, color),
			ValueOrDefault(yA1100, color),
			ValueOrDefault(yA1101, color),
			ValueOrDefault(yA1110, color),
			ValueOrDefault(yA1111, color)
		};

		outColor->R = (wSum * 16 - w[0] * y[0].R + w[1] * y[1].R + w[2] * y[2].R + w[3] * y[3].R + w[4] * y[4].R + w[5] * y[5].R + w[6] * y[6].R + w[7] * y[7].R + w[8] * y[8].R + w[9] * y[9].R + w[10] * y[10].R + w[11] * y[11].R + w[12] * y[12].R + w[13] * y[13].R + w[14] * y[14].R + w[15] * y[15].R) / wSum;
		outColor->G = (wSum * 16 - w[0] * y[0].G + w[1] * y[1].G + w[2] * y[2].G + w[3] * y[3].G + w[4] * y[4].G + w[5] * y[5].G + w[6] * y[6].G + w[7] * y[7].G + w[8] * y[8].G + w[9] * y[9].G + w[10] * y[10].G + w[11] * y[11].G + w[12] * y[12].G + w[13] * y[13].G + w[14] * y[14].G + w[15] * y[15].G) / wSum;
		outColor->B = (wSum * 16 - w[0] * y[0].B + w[1] * y[1].B + w[2] * y[2].B + w[3] * y[3].B + w[4] * y[4].B + w[5] * y[5].B + w[6] * y[6].B + w[7] * y[7].B + w[8] * y[8].B + w[9] * y[9].B + w[10] * y[10].B + w[11] * y[11].B + w[12] * y[12].B + w[13] * y[13].B + w[14] * y[14].B + w[15] * y[15].B) / wSum;
		outColor->A = (wSum * 16 - w[0] * y[0].A + w[1] * y[1].A + w[2] * y[2].A + w[3] * y[3].A + w[4] * y[4].A + w[5] * y[5].A + w[6] * y[6].A + w[7] * y[7].A + w[8] * y[8].A + w[9] * y[9].A + w[10] * y[10].A + w[11] * y[11].A + w[12] * y[12].A + w[13] * y[13].A + w[14] * y[14].A + w[15] * y[15].A) / wSum;
	}

	return true;
}

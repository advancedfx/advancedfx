#include "stdafx.h"

#include "AfxColorLut.h"

#include <shared/AfxMath.h>

#include <algorithm>


const char CAfxColorLut::m_Magic[11] = { 'A','f','x','R','g','b','a','L','u','t','\0' };

bool CAfxColorLut::Query(float r, float g, float b, float a, float& outR, float& outG, float& outB, float& outA)
{
	if (nullptr == m_Root) return false;

	size_t resR = m_Dimensions.GetSize();
	size_t resG = m_Dimensions.GetSub().GetSize();
	size_t resB = m_Dimensions.GetSub().GetSub().GetSize();
	size_t resA = m_Dimensions.GetSub().GetSub().GetSub().GetSize();

	float xR0, xR1;
	GreenLookupTreeNode_t* yR0, * yR1;
	GetInterval<RedLookupTreeNode_t, GreenLookupTreeNode_t>(m_Root, resR, r, xR0, xR1, yR0, yR1);

	float xG00, xG01, xG10, xG11;
	BlueLookupTreeNode_t* yG00, * yG01, * yG10, * yG11;
	GetInterval<GreenLookupTreeNode_t, BlueLookupTreeNode_t>(yR0, resG, g, xG00, xG01, yG00, yG01);
	GetInterval<GreenLookupTreeNode_t, BlueLookupTreeNode_t>(yR1, resG, g, xG10, xG11, yG10, yG11);

	float xB000, xB001, xB010, xB011, xB100, xB101, xB110, xB111;
	AlphaLookupTreeNode_t* yB000, * yB001, * yB010, * yB011, * yB100, * yB101, * yB110, * yB111;
	GetInterval<BlueLookupTreeNode_t, AlphaLookupTreeNode_t>(yG00, resB, b, xB000, xB001, yB000, yB001);
	GetInterval<BlueLookupTreeNode_t, AlphaLookupTreeNode_t>(yG01, resB, b, xB010, xB011, yB010, yB011);
	GetInterval<BlueLookupTreeNode_t, AlphaLookupTreeNode_t>(yG10, resB, b, xB100, xB101, yB100, yB101);
	GetInterval<BlueLookupTreeNode_t, AlphaLookupTreeNode_t>(yG11, resB, b, xB110, xB111, yB110, yB111);

	float xA0000, xA0001, xA0010, xA0011, xA0100, xA0101, xA0110, xA0111, xA1000, xA1001, xA1010, xA1011, xA1100, xA1101, xA1110, xA1111;
	CRgbaUc* yA0000, * yA0001, * yA0010, * yA0011, * yA0100, * yA0101, * yA0110, * yA0111, * yA1000, * yA1001, * yA1010, * yA1011, * yA1100, * yA1101, * yA1110, * yA1111;
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB000, resA, a, xA0000, xA0001, yA0000, yA0001);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB001, resA, a, xA0010, xA0011, yA0010, yA0011);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB010, resA, a, xA0100, xA0101, yA0100, yA0101);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB011, resA, a, xA0110, xA0111, yA0110, yA0111);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB100, resA, a, xA1000, xA1001, yA1000, yA1001);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB101, resA, a, xA1010, xA1011, yA1010, yA1011);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB110, resA, a, xA1100, xA1101, yA1100, yA1101);
	GetInterval<AlphaLookupTreeNode_t, CRgbaUc>(yB111, resA, a, xA1110, xA1111, yA1110, yA1111);

	{
		CRgba x(r, g, b, a);

		CRgba y[16] = {
			ValueOrDefault(yA0000, x),
			ValueOrDefault(yA0001, x),
			ValueOrDefault(yA0010, x),
			ValueOrDefault(yA0011, x),
			ValueOrDefault(yA0100, x),
			ValueOrDefault(yA0101, x),
			ValueOrDefault(yA0110, x),
			ValueOrDefault(yA0111, x),
			ValueOrDefault(yA1000, x),
			ValueOrDefault(yA1001, x),
			ValueOrDefault(yA1010, x),
			ValueOrDefault(yA1011, x),
			ValueOrDefault(yA1100, x),
			ValueOrDefault(yA1101, x),
			ValueOrDefault(yA1110, x),
			ValueOrDefault(yA1111, x)
		};

		CRgba yB000 = CRgba::Interp(x, CRgba(xR0, xG00, xB000, xA0000), CRgba(xR0, xG00, xB000, xA0001), y[0b0000], y[0b0001]);
		CRgba yB001 = CRgba::Interp(x, CRgba(xR0, xG00, xB001, xA0010), CRgba(xR0, xG00, xB001, xA0011), y[0b0010], y[0b0011]);
		CRgba yB010 = CRgba::Interp(x, CRgba(xR0, xG01, xB010, xA0100), CRgba(xR0, xG01, xB010, xA0101), y[0b0100], y[0b0101]);
		CRgba yB011 = CRgba::Interp(x, CRgba(xR0, xG01, xB011, xA0110), CRgba(xR0, xG01, xB011, xA0111), y[0b0110], y[0b0111]);
		CRgba yB100 = CRgba::Interp(x, CRgba(xR1, xG10, xB100, xA1000), CRgba(xR1, xG10, xB100, xA1001), y[0b1000], y[0b1001]);
		CRgba yB101 = CRgba::Interp(x, CRgba(xR1, xG10, xB101, xA1010), CRgba(xR1, xG10, xB101, xA1011), y[0b1010], y[0b1011]);
		CRgba yB110 = CRgba::Interp(x, CRgba(xR1, xG11, xB110, xA1100), CRgba(xR1, xG11, xB110, xA1101), y[0b1100], y[0b1101]);
		CRgba yB111 = CRgba::Interp(x, CRgba(xR1, xG11, xB111, xA1110), CRgba(xR1, xG11, xB111, xA1111), y[0b1110], y[0b1111]);

		CRgba yG00 = CRgba::Interp(x, CRgba(xR0, xG00, xB000, x.A), CRgba(xR0, xG00, xB001, x.A), yB000, yB001);
		CRgba yG01 = CRgba::Interp(x, CRgba(xR0, xG01, xB010, x.A), CRgba(xR0, xG01, xB011, x.A), yB010, yB011);
		CRgba yG10 = CRgba::Interp(x, CRgba(xR1, xG10, xB100, x.A), CRgba(xR1, xG10, xB101, x.A), yB100, yB101);
		CRgba yG11 = CRgba::Interp(x, CRgba(xR1, xG11, xB110, x.A), CRgba(xR1, xG11, xB111, x.A), yB110, yB111);

		CRgba yR0 = CRgba::Interp(x, CRgba(xR0, xG00, x.B, x.A), CRgba(xR0, xG01, x.B, x.A), yG00, yG01);
		CRgba yR1 = CRgba::Interp(x, CRgba(xR1, xG10, x.B, x.A), CRgba(xR1, xG11, x.B, x.A), yG10, yG11);

		CRgba outY = CRgba::Interp(x, CRgba(xR0, x.G, x.B, x.A), CRgba(xR1, x.G, x.B, x.A), yR0, yR1);

		outR = outY.R;
		outG = outY.G;
		outB = outY.B;
		outA = outY.A;
	}

	return true;
}

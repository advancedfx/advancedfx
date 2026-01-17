#pragma once

#include "SourceInterfaces.h"

#ifndef _WIN64
#include "MirvCalcs.h"
#endif //#ifndef _WIN64

#include <string>

class CMirvCam
{
public:
	enum Origin_e {
		O_Net,
		O_View
	};

	enum Angles_e {
		A_Net,
		A_View
	};

	/// <returns>true if overriden</returns>
	bool ApplySource(float & x, float & y, float & z, float & xRotation, float & yRotation, float & zRotation);

	/// <returns>true if overriden</returns>
	bool ApplyOffset(float & x, float & y, float & z, float & xRotation, float & yRotation, float & zRotation);

	/// <returns>true if overriden</returns>
	bool ApplyFov(float & fov);

public:
	SOURCESDK::CSGO::CBaseHandle m_SourceHandle;
	Origin_e m_SourceOrigin = O_View;
	Angles_e m_SourceAngles = A_View;
	std::string m_SourceAttachment;

#ifndef _WIN64
	void RebuildCalc(void);
#endif //#ifndef _WIN64

	bool m_SourceUseX = true;
	bool m_SourceUseY = true;
	bool m_SourceUseZ = true;
	bool m_SourceUseXRotation = true;
	bool m_SourceUseYRotation = true;
	bool m_SourceUseZRotation = true;
	float m_OffsetForwad = 0;
	float m_OffsetLeft = 0;
	float m_OffsetUp = 0;
	float m_OffsetForwardRot = 0;
	float m_OffseLeftRot = 0;
	float m_OffsetUpRot = 0;

	~CMirvCam()
	{
#ifndef _WIN64	
		Source_set(0);
#endif //#ifndef _WIN64	
	}

#ifndef _WIN64
	IMirvVecAngCalc * Source_get(void)
	{
		return m_Source;
	}

	void Source_set(IMirvVecAngCalc * value)
	{
		if (m_Source) m_Source->Release();
		m_Source = value;
		if (m_Source) m_Source->AddRef();
	}

	IMirvFovCalc * Fov_get(void)
	{
		return m_Fov;
	}

	void Fov_set(IMirvFovCalc * value)
	{
		if (m_Fov) m_Fov->Release();
		m_Fov = value;
		if (m_Fov) m_Fov->AddRef();
	}
#endif //#ifndef _WIN64

private:
#ifndef _WIN64
	IMirvVecAngCalc * m_Source = 0;
	IMirvFovCalc * m_Fov = 0;
#endif //#ifndef _WIN64
};

extern CMirvCam g_MirvCam;

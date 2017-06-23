#include "stdafx.h"

#include "MirvCam.h"

#include "WrpVEngineClient.h"
#include "WrpConsole.h"

#include <shared/AfxMath.h>


void CMirvCam::ApplySource(float & x, float & y, float & z, float & xRotation, float & yRotation, float & zRotation)
{
	if (m_Source)
	{
		SOURCESDK::Vector o;
		SOURCESDK::QAngle a;

		if (!m_Source->CalcVecAng(o, a))
		{
			o.x = x;
			o.y = y;
			o.z = z;
			a.x = xRotation;
			a.y = yRotation;
			a.z = zRotation;
		}

		if (m_SourceUseX) x = o.x;
		if (m_SourceUseY) y = o.y;
		if (m_SourceUseZ) z = o.z;

		if (m_SourceUseXRotation) xRotation = a.z;
		if (m_SourceUseYRotation) yRotation = a.x;
		if (m_SourceUseZRotation) zRotation = a.y;
	}
}

void CMirvCam::ApplyOffset(float & x, float & y, float & z, float & xRotation, float & yRotation, float & zRotation)
{
	if (m_OffsetForwad || m_OffsetLeft || m_OffsetUp)
	{
		double forward[3], right[3], up[3];

		Afx::Math::MakeVectors(xRotation, yRotation, zRotation, forward, right, up);

		x = (float)(x + m_OffsetForwad*forward[0] - m_OffsetLeft*right[0] + m_OffsetUp*up[0]);
		y = (float)(y + m_OffsetForwad*forward[1] - m_OffsetLeft*right[1] + m_OffsetUp*up[1]);
		z = (float)(z + m_OffsetForwad*forward[2] - m_OffsetLeft*right[2] + m_OffsetUp*up[2]);
	}

	if (m_OffsetForwardRot || m_OffseLeftRot || m_OffsetUpRot)
	{
		Afx::Math::Quaternion q1 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(yRotation, zRotation, xRotation)));
		Afx::Math::Quaternion q2 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(m_OffseLeftRot, m_OffsetUpRot, m_OffsetForwardRot)));

		Afx::Math::QEulerAngles angs = (q1 * q2).ToQREulerAngles().ToQEulerAngles();

		xRotation = (float)angs.Roll;
		yRotation = (float)angs.Pitch;
		zRotation = (float)angs.Yaw;
	}
}

void CMirvCam::RebuildCalc(void)
{
	IMirvHandleCalc * handleCalc = g_MirvHandleCalcs.NewValueCalc(0, m_SourceHandle.ToInt());
	if (handleCalc)
	{
		handleCalc->AddRef();

		if (!m_SourceAttachment.empty())
		{
			Source_set(
				g_MirvVecAngCalcs.NewHandleAttachmentCalc(0, handleCalc, m_SourceAttachment.c_str())
			);
		}
		else
		{
			Source_set(
				g_MirvVecAngCalcs.NewHandleCalcEx(0, handleCalc, O_View == m_SourceOrigin, A_View == m_SourceAngles)
			);
		}

		handleCalc->Release();
	}
}

CMirvCam g_MirvCam;

CON_COMMAND(mirv_cam, "Control camera source entity and offset.")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("source", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("calcVecAng", arg2))
				{
					if (4 <= argc)
					{
						IMirvVecAngCalc * vecAng = g_MirvVecAngCalcs.GetByName(args->ArgV(3));

						if (!vecAng)
							Tier0_Warning("No vecAng calc \"%s\" exists.\n", args->ArgV(3));

						g_MirvCam.Source_set(vecAng);

						return;
					}

					IMirvVecAngCalc * vecAng = g_MirvCam.Source_get();

					Tier0_Msg(
						"mirv_cam source calcVecAng <sVecAngCalcName> - Calc to use as source (<sVecAngCalcName> is name form mirv_calcs vecAng).\n"
						"Cuurent value: %s"
						, vecAng ? "" : "(none)"
					);

					if (vecAng) vecAng->Console_Print();
					Tier0_Msg("\n");

					return;
				}
				else if (0 == _stricmp("calcVecAngClear", arg2))
				{
					g_MirvCam.Source_set(0);	
				}
				else if (0 == _stricmp("handle", arg2))
				{
					if (4 <= argc)
					{
						char const * arg3 = args->ArgV(3);

						if (0 == _stricmp("none", arg3))
							g_MirvCam.m_SourceHandle = SOURCESDK_CSGO_INVALID_EHANDLE_INDEX;
						else
							g_MirvCam.m_SourceHandle = atoi(arg3);

						g_MirvCam.RebuildCalc();

						return;
					}

					Tier0_Msg(
						"mirv_cam source handle none|<n> - Handle to use as source, use mirv_listentites to find the entityHandle or mirv_streams ... picker.\n"
						"Handle: %i%s\n"
						, g_MirvCam.m_SourceHandle.ToInt()
						, g_MirvCam.m_SourceHandle.IsValid() ? "" : " (none)"
					);

					return;
				}
				else if (0 == _stricmp("origin", arg2))
				{
					if (4<= argc)
					{
						char const * arg3 = args->ArgV(3);

						if (0 == _stricmp("net", arg3))
						{
							g_MirvCam.m_SourceOrigin = CMirvCam::O_Net;

							g_MirvCam.RebuildCalc();

							return;
						}
						else
						if (0 == _stricmp("view", arg3))
						{
							g_MirvCam.m_SourceOrigin = CMirvCam::O_View;

							g_MirvCam.RebuildCalc();

							return;
						}
					}

					char const * curValue = "[unknown]";
					switch (g_MirvCam.m_SourceOrigin)
					{
					case CMirvCam::O_Net:
						curValue = "net";
						break;
					case CMirvCam::O_View:
						curValue = "view";
						break;
					}

					Tier0_Msg(
						"mirv_cam source origin net|view - Source origin to use.\n"
						"Current value: %s\n",
						curValue
					);
					return;
				}
				else if (0 == _stricmp("angles", arg2))
				{
					if (4 <= argc)
					{
						char const * arg3 = args->ArgV(3);

						if (0 == _stricmp("net", arg3))
						{
							g_MirvCam.m_SourceAngles = CMirvCam::A_Net;

							g_MirvCam.RebuildCalc();

							return;
						}
						else
							if (0 == _stricmp("view", arg3))
							{
								g_MirvCam.m_SourceAngles = CMirvCam::A_View;

								g_MirvCam.RebuildCalc();

								return;
							}
					}

					char const * curValue = "[unknown]";
					switch (g_MirvCam.m_SourceAngles)
					{
					case CMirvCam::A_Net:
						curValue = "net";
						break;
					case CMirvCam::A_View:
						curValue = "view";
						break;
					}

					Tier0_Msg(
						"mirv_cam source angles net|view - Source angeles to use.\n"
						"Current value: %s\n",
						curValue
					);
					return;
				}
				else if (0 == _stricmp("attachment", arg2))
				{
					if (4 <= argc)
					{
						g_MirvCam.m_SourceAttachment = args->ArgV(3);

						g_MirvCam.RebuildCalc();

						return;
					}

					Tier0_Msg(
						"mirv_cam source attachment <sName> - use i.e. \"muzzle_flash\" for 3rd person weapon entites for great results.\n"
						"Current value: %s\n"
						, g_MirvCam.m_SourceAttachment.c_str()
					);
					return;
				}
				else if (0 == _stricmp("attachmentNone", arg2))
				{
					g_MirvCam.m_SourceAttachment.clear();

					g_MirvCam.RebuildCalc();

					return;
				}
				else if (0 == _stricmp("originUse", arg2))
				{
					if (6 <= argc)
					{
						g_MirvCam.m_SourceUseX = 0 != atoi(args->ArgV(3));
						g_MirvCam.m_SourceUseY = 0 != atoi(args->ArgV(4));
						g_MirvCam.m_SourceUseZ = 0 != atoi(args->ArgV(5));
						return;
					}

					Tier0_Msg(
						"mirv_cam source originUse <iUseX> <iUseY> <iUseZ> - Use 0 for don't use, 1 for use.\n"
						"Current value: %i %i %i\n"
						, g_MirvCam.m_SourceUseX ? 1 : 0
						, g_MirvCam.m_SourceUseY ? 1 : 0
						, g_MirvCam.m_SourceUseZ ? 1 : 0
					);
					return;
				}
				else if (0 == _stricmp("anglesUse", arg2))
				{
					if (6 <= argc)
					{
						g_MirvCam.m_SourceUseXRotation = 0 != atoi(args->ArgV(3));
						g_MirvCam.m_SourceUseYRotation = 0 != atoi(args->ArgV(4));
						g_MirvCam.m_SourceUseZRotation = 0 != atoi(args->ArgV(5));
						return;
					}

					Tier0_Msg(
						"mirv_cam source anglesUse <iUseRotX> <iUseRotY> <iUseRotZ> - Use 0 for don't use, 1 for use.\n"
						"Current value: %i %i %i\n"
						, g_MirvCam.m_SourceUseXRotation ? 1 : 0
						, g_MirvCam.m_SourceUseYRotation ? 1 : 0
						, g_MirvCam.m_SourceUseZRotation ? 1 : 0
					);
					return;
				}
			}

			Tier0_Msg(
				"mirv_cam source calcVecAng [...] - Calc to use as source (overrides handle, origin, angles, attachment, attachmentNone).\n"
				"mirv_cam source calcVecAngClear - Clear source (overrides handle, origin, angles, attachment, attachmentNone).\n"
				"mirv_cam source handle [...] - Entity handle to use as source.\n"
				"mirv_cam source origin [...] - Controls source origin type.\n"
				"mirv_cam source angles [...] - Controls source angles type.\n"
				"mirv_cam source attachment [...] - Controls if and what attachment to use as source.\n"
				"mirv_cam source attachmentNone - Use no attachment.\n"
				"mirv_cam source originUse [...] - Controls which components of source origin to use.\n"
				"mirv_cam source anglesUse [...] - Controls which components of source angles to use.\n"
			);
			return;
		}
		else if (0 == _stricmp("offset", arg1))
		{
			if (5 <= argc)
			{
				g_MirvCam.m_OffsetForwad = (float)atof(args->ArgV(2));
				g_MirvCam.m_OffsetLeft = (float)atof(args->ArgV(3));
				g_MirvCam.m_OffsetUp = (float)atof(args->ArgV(4));

				if (8 <= argc)
				{
					g_MirvCam.m_OffsetForwardRot = (float)atof(args->ArgV(5));
					g_MirvCam.m_OffseLeftRot = (float)atof(args->ArgV(6));
					g_MirvCam.m_OffsetUpRot = (float)atof(args->ArgV(7));
				}
				return;
			}
			else if (3 <= argc && 0 == _stricmp("none", args->ArgV(2)))
			{
				g_MirvCam.m_OffsetForwad = 0;
				g_MirvCam.m_OffsetLeft = 0;
				g_MirvCam.m_OffsetUp = 0;

				g_MirvCam.m_OffsetForwardRot = 0;
				g_MirvCam.m_OffseLeftRot = 0;
				g_MirvCam.m_OffsetUpRot =0;

				return;
			}


			Tier0_Msg(
				"mirv_cam offset none - No camera offset.\n"
				"mirv_cam offset <fXForward> <fYLeft> <fZUp> [<fRotXForward> <fRotYLeft> <fRotZUp>] - Camera offset location, rotation optional, floating point values.\n"
				"Current value: <fXForward>=%f <fYLeft>=%f <fZUp>=%f <fRotXForward>=%f <fRotYLeft>=%f <fRotZUp>=%f\n"
				, g_MirvCam.m_OffsetForwad, g_MirvCam.m_OffsetLeft, g_MirvCam.m_OffsetUp
				, g_MirvCam.m_OffsetForwardRot, g_MirvCam.m_OffseLeftRot, g_MirvCam.m_OffsetUpRot
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_cam source [...] - Control camera source entity.\n"
		"mirv_cam offset [...] - Control camera offset (in local space).\n"
	);
}

#include "stdafx.h"

#include "GameRecord.h"

#include <hlsdk.h>
#include <deps/release/halflife/common/r_studioint.h>

#include "hooks/hw/Host_Frame.h"
#include "hooks/hw/R_RenderView.h"
#include "cmdregister.h"
#include "hl_addresses.h"

#include <shared/StringTools.h>
#include <shared/AfxMath.h>

#include <string>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

class CGameRecord g_GameRecord;

extern cl_enginefuncs_s* pEngfuncs;
extern engine_studio_api_s* pEngStudio;

extern double * g_phost_frametime;

bool g_bStudioWasPlayer = false;

typedef void (*R_RestoreRenderer_t)(void);

R_RestoreRenderer_t g_OldRestoreRenderer = nullptr;

void MyRestoreRenderer(void)
{
	g_GameRecord.RecordCurrentEntity();
	g_OldRestoreRenderer();
}

bool Hook_R_RestoreRenderer()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	struct engine_studio_api_s* pstudio = (struct engine_studio_api_s*)HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio);

	if (!pstudio) {
		firstResult = false;
	}
	else
	{
		LONG error = NO_ERROR;

		g_OldRestoreRenderer = pstudio->RestoreRenderer;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_OldRestoreRenderer, MyRestoreRenderer);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_RestoreRenderer");
		}
	}

	return firstResult;
}

bool CGameRecord::HooksValid() {
	return
		1 == HL_ADDR_GET(hw_HUD_GetStudioModelInterface_version)
		&& HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pInterface)
		&& HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio)
		&& Hook_Host_Frame()
		&& Hook_R_RenderView()
		&& Hook_R_RestoreRenderer()
	;
}

bool CGameRecord::GetRecording() {
	return m_AfxGameRecord.GetRecording();
}

bool CGameRecord::StartRecording(const char* fileName)
{
	std::wstring strFileName;

	if (!UTF8StringToWideString(fileName, strFileName))
	{
		pEngfuncs->Con_Printf("AfxError: could not convert \"%s\" from UTF8 to wchar.\n", fileName);
		return false;
	}

	if (!m_AfxGameRecord.StartRecording(strFileName.c_str(), 5))
	{
		pEngfuncs->Con_Printf("AfxError: could not start recording to \"%s\".\n", fileName);
		return false;
	}

	return true;
}

void CGameRecord::EndRecording()
{
	m_AfxGameRecord.EndRecording();
}

void CGameRecord::OnFrameBegin()
{
	if (!GetRecording()) return;

	m_AfxGameRecord.BeginFrame((float)*g_phost_frametime);
}

void CGameRecord::OnFrameEnd(float view_origin[3], float view_angles[3], float fov)
{
	if (!GetRecording()) return;

	if (RecordCamera)
	{
		m_AfxGameRecord.WriteDictionary("afxCam");
		m_AfxGameRecord.Write((float)view_origin[0]);
		m_AfxGameRecord.Write((float)view_origin[1]);
		m_AfxGameRecord.Write((float)view_origin[2]);
		m_AfxGameRecord.Write((float)view_angles[0]);
		m_AfxGameRecord.Write((float)view_angles[1]);
		m_AfxGameRecord.Write((float)view_angles[2]);
		m_AfxGameRecord.Write((float)fov);
	}

	m_AfxGameRecord.EndFrame();
}

void CGameRecord::RecordCurrentEntity()
{
	struct engine_studio_api_s* pstudio = (struct engine_studio_api_s*)HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio);

	if (cl_entity_s* ent = pstudio->GetCurrentEntity())
	{
		m_AfxGameRecord.WriteDictionary("entity_state");
		m_AfxGameRecord.Write((int)ent->index);

		if (model_t* model = ent->model)
		{
			m_AfxGameRecord.WriteDictionary("baseentity");
			//Write((float)pBaseEntityRs->m_flTime);
			m_AfxGameRecord.WriteDictionary(ent->model->name);
			m_AfxGameRecord.Write((bool)true);
			WriteVector(ent->origin);
			WriteQAngle(ent->angles);

			if (studiohdr_t* header = (studiohdr_t*)pstudio->Mod_Extradata(model))
			{
				int numbones = header->numbones;

				mstudiobone_t * pbones = (mstudiobone_t*)((byte*)header + header->boneindex);

				m_AfxGameRecord.WriteDictionary("baseanimating");
				//Write((int)pBaseAnimatingRs->m_nSkin);
				//Write((int)pBaseAnimatingRs->m_nBody);
				//Write((int)pBaseAnimatingRs->m_nSequence);

				float(*pBoneM)[MAXSTUDIOBONES][3][4] = (float(*)[MAXSTUDIOBONES][3][4])(pstudio->StudioGetBoneTransform());
				float(*pRotM)[3][4] = (float(*)[3][4])(pstudio->StudioGetRotationMatrix());

				if (0 < numbones && pBoneM && pRotM)
				{
					double M[4][4] = {
						(*pRotM)[0][0], (*pRotM)[0][1], (*pRotM)[0][2],  (*pRotM)[0][3],
						(*pRotM)[1][0], (*pRotM)[1][1], (*pRotM)[1][2],  (*pRotM)[1][3],
						(*pRotM)[2][0], (*pRotM)[2][1], (*pRotM)[2][2],  (*pRotM)[2][3],
						0, 0, 0, 1
					};

					double b0[4] = { 1, 0, 0, 0 };
					double b1[4] = { 0, 1, 0, 0 };
					double b2[4] = { 0, 0, 1, 0 };
					double b3[4] = { 0, 0, 0, 1 };

					unsigned char P[4];
					unsigned char Q[4];

					double L[4][4];
					double U[4][4];

					if (Afx::Math::LUdecomposition(M, P, Q, L, U))
					{
						double inv0[4] = { 1,0,0,0 };
						double inv1[4] = { 0,1,0,0 };
						double inv2[4] = { 0,0,1,0 };
						double inv3[4] = { 0,0,0,1 };

						Afx::Math::SolveWithLU(L, U, P, Q, b0, inv0);
						Afx::Math::SolveWithLU(L, U, P, Q, b1, inv1);
						Afx::Math::SolveWithLU(L, U, P, Q, b2, inv2);
						Afx::Math::SolveWithLU(L, U, P, Q, b3, inv3);

						float newMatrix[4][4] = {
							(float)inv0[0], (float)inv0[1], (float)inv0[2], (float)inv0[3],
							(float)inv1[0], (float)inv1[1], (float)inv1[2], (float)inv1[3],
							(float)inv2[0], (float)inv2[1], (float)inv2[2], (float)inv2[3],
							(float)inv3[0], (float)inv3[1], (float)inv3[2], (float)inv3[3]
						};

						m_AfxGameRecord.Write((bool)true);

						m_AfxGameRecord.Write((int)header->numbones);

						for (int i = 0; i < header->numbones; ++i)
						{
							float boneMatrix[4][4] = {
								(*pBoneM)[i][0][0] * newMatrix[0][0] + (*pBoneM)[i][0][1] * newMatrix[1][0] + (*pBoneM)[i][0][2] * newMatrix[2][0] + 0 * newMatrix[3][0],
								(*pBoneM)[i][0][0] * newMatrix[0][1] + (*pBoneM)[i][0][1] * newMatrix[1][1] + (*pBoneM)[i][0][2] * newMatrix[2][1] + 0 * newMatrix[3][1],
								(*pBoneM)[i][0][0] * newMatrix[0][2] + (*pBoneM)[i][0][1] * newMatrix[1][2] + (*pBoneM)[i][0][2] * newMatrix[2][2] + 0 * newMatrix[3][2],
								(*pBoneM)[i][0][0] * newMatrix[0][3] + (*pBoneM)[i][0][1] * newMatrix[1][3] + (*pBoneM)[i][0][2] * newMatrix[2][3] + (*pBoneM)[i][0][3] * newMatrix[3][3],

								(*pBoneM)[i][1][0] * newMatrix[0][0] + (*pBoneM)[i][1][1] * newMatrix[1][0] + (*pBoneM)[i][1][2] * newMatrix[2][0] + 0 * newMatrix[3][0],
								(*pBoneM)[i][1][0] * newMatrix[0][1] + (*pBoneM)[i][1][1] * newMatrix[1][1] + (*pBoneM)[i][1][2] * newMatrix[2][1] + 0 * newMatrix[3][1],
								(*pBoneM)[i][1][0] * newMatrix[0][2] + (*pBoneM)[i][1][1] * newMatrix[1][2] + (*pBoneM)[i][1][2] * newMatrix[2][2] + 0 * newMatrix[3][2],
								(*pBoneM)[i][1][0] * newMatrix[0][3] + (*pBoneM)[i][1][1] * newMatrix[1][3] + (*pBoneM)[i][1][2] * newMatrix[2][3] + (*pBoneM)[i][1][3] * newMatrix[3][3],

								(*pBoneM)[i][2][0] * newMatrix[0][0] + (*pBoneM)[i][2][1] * newMatrix[1][0] + (*pBoneM)[i][2][2] * newMatrix[2][0] + 0 * newMatrix[3][0],
								(*pBoneM)[i][2][0] * newMatrix[0][1] + (*pBoneM)[i][2][1] * newMatrix[1][1] + (*pBoneM)[i][2][2] * newMatrix[2][1] + 0 * newMatrix[3][1],
								(*pBoneM)[i][2][0] * newMatrix[0][2] + (*pBoneM)[i][2][1] * newMatrix[1][2] + (*pBoneM)[i][2][2] * newMatrix[2][2] + 0 * newMatrix[3][2],
								(*pBoneM)[i][2][0] * newMatrix[0][3] + (*pBoneM)[i][2][1] * newMatrix[1][3] + (*pBoneM)[i][2][2] * newMatrix[2][3] + (*pBoneM)[i][2][3] * newMatrix[3][3],

								0 * newMatrix[0][0] + 0 * newMatrix[1][0] + 0 * newMatrix[2][0] + 1 * newMatrix[3][0],
								0 * newMatrix[0][1] + 0 * newMatrix[1][1] + 0 * newMatrix[2][1] + 1 * newMatrix[3][1],
								0 * newMatrix[0][2] + 0 * newMatrix[1][2] + 0 * newMatrix[2][2] + 1 * newMatrix[3][2],
								0 * newMatrix[0][3] + 0 * newMatrix[1][3] + 0 * newMatrix[2][3] + 1 * newMatrix[3][3]
							};

							float pos[3] = {
								boneMatrix[0][3],
								boneMatrix[1][3],
								boneMatrix[2][3]
							};
							WriteVector(pos);

							// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/

							float tr = boneMatrix[0][0] + boneMatrix[1][1] + boneMatrix[2][2];

							float qw = 1;
							float qx = 0;
							float qy = 0;
							float qz = 0;

							if (tr > 0) {
								float S = sqrt(tr + 1.0) * 2; // S=4*qw 
								qw = 0.25 * S;
								qx = (boneMatrix[1][2] - boneMatrix[2][1]) / S;
								qy = (boneMatrix[2][0] - boneMatrix[0][2]) / S;
								qz = (boneMatrix[1][0] - boneMatrix[0][1]) / S;
							}
							else if ((boneMatrix[0][0] > boneMatrix[1][1]) & (boneMatrix[0][0] > boneMatrix[2][2])) {
								float S = sqrt(1.0 + boneMatrix[0][0] - boneMatrix[1][1] - boneMatrix[2][2]) * 2; // S=4*qx 
								qw = (boneMatrix[1][2] - boneMatrix[2][1]) / S;
								qx = 0.25 * S;
								qy = (boneMatrix[1][0] + boneMatrix[0][1]) / S;
								qz = (boneMatrix[2][0] + boneMatrix[0][2]) / S;
							}
							else if (boneMatrix[1][1] > boneMatrix[2][2]) {
								float S = sqrt(1.0 + boneMatrix[1][1] - boneMatrix[0][0] - boneMatrix[2][2]) * 2; // S=4*qy
								qw = (boneMatrix[2][0] - boneMatrix[0][2]) / S;
								qx = (boneMatrix[1][0] + boneMatrix[0][1]) / S;
								qy = 0.25 * S;
								qz = (boneMatrix[2][1] + boneMatrix[1][2]) / S;
							}
							else {
								float S = sqrt(1.0 + boneMatrix[2][2] - boneMatrix[0][0] - boneMatrix[1][1]) * 2; // S=4*qz
								qw = (boneMatrix[0][1] - boneMatrix[1][0]) / S;
								qx = (boneMatrix[2][0] + boneMatrix[0][2]) / S;
								qy = (boneMatrix[2][1] + boneMatrix[1][2]) / S;
								qz = 0.25 * S;
							}

							float quat[4] = {
								qx, qy, qz, qw							
							};
							WriteQuaternion(quat);
						}
					}
					else
					{
						m_AfxGameRecord.Write((bool)false);
					}
				}
			}
		}

		m_AfxGameRecord.WriteDictionary("/");

		bool viewModel = false;

		m_AfxGameRecord.Write((bool)viewModel);
	}
}

void CGameRecord::WriteVector(float  value[3])
{
	m_AfxGameRecord.Write(value[0]);
	m_AfxGameRecord.Write(value[1]);
	m_AfxGameRecord.Write(value[2]);
}

void CGameRecord::WriteQAngle(float value[3])
{
	m_AfxGameRecord.Write(value[0]);
	m_AfxGameRecord.Write(value[1]);
	m_AfxGameRecord.Write(value[2]);
}

void CGameRecord::WriteQuaternion(float value[4])
{
	m_AfxGameRecord.Write(value[0]);
	m_AfxGameRecord.Write(value[1]);
	m_AfxGameRecord.Write(value[2]);
	m_AfxGameRecord.Write(value[3]);
}

REGISTER_CMD_FUNC(agr)
{
	if (!g_GameRecord.HooksValid())
	{
		pEngfuncs->Con_Printf("AFXERROR: Missing hooks.\n");
		return;
	}

	int argc = pEngfuncs->Cmd_Argc();

	if (argc < 1) return;

	char const* prefix = pEngfuncs->Cmd_Argv(0);

	if (2 <= argc)
	{
		char const* cmd1 = pEngfuncs->Cmd_Argv(1);

		if (!_stricmp(cmd1, "start"))
		{
			if (3 <= argc)
			{
				char const* cmd2 = pEngfuncs->Cmd_Argv(2);

				std::wstring wideFilePath;

				g_GameRecord.StartRecording(cmd2);

				if (g_GameRecord.GetRecording())
				{
					pEngfuncs->Con_Printf("Started AGR recording.\n");
					return;
				}

				pEngfuncs->Con_Printf(
					"Error.\n"
				);
				return;
			}
		}
		else if (!_stricmp(cmd1, "stop"))
		{
			g_GameRecord.EndRecording();

			pEngfuncs->Con_Printf(
				"Stopped AGR recording.\n"
			);
			return;
		}
	}

	pEngfuncs->Con_Printf(
		"%s start <sFilePath> - Start recording to file <sFilePath>, you should set a low host_framerate before (i.e. 30) and give the \".agr\" file extension.\n"
		"%s stop - Stop recording.\n"
		, prefix
		, prefix
	);

}
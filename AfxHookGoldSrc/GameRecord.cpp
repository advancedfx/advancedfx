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
#include <cmath>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#undef vec3_t
extern "C" {
#include <deps/release/halflife/utils/common/mathlib.h>
}

class CGameRecord g_GameRecord;

extern cl_enginefuncs_s* pEngfuncs;
extern engine_studio_api_s* pEngStudio;

extern double * g_phost_frametime;


typedef void	(*R_SetRenderModel_t)(struct model_s* model);

R_SetRenderModel_t g_OldSetRenderModel = nullptr;

void MySetRenderModel(struct model_s* model)
{
	g_OldSetRenderModel(model);
	g_GameRecord.SetRenderModel(model);
}


bool Hook_R_SetRenderModel()
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

		g_OldSetRenderModel = pstudio->SetRenderModel;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_OldSetRenderModel, MySetRenderModel);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_SetRenderModel");
		}
	}

	return firstResult;
}


typedef void	(*R_StudioSetHeader_t)(void* header);

R_StudioSetHeader_t g_OldStudioSetHeader = nullptr;

void MyStudioSetHeader(void* header)
{
	g_OldStudioSetHeader(header);
	g_GameRecord.StudioSetHeader(header);
}

bool Hook_R_StudioSetHeader()
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

		g_OldStudioSetHeader = pstudio->StudioSetHeader;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_OldStudioSetHeader, MyStudioSetHeader);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_StudioSetHeader");
		}
	}

	return firstResult;
}

typedef void (*SetupRenderer_t)(int rendermode);

SetupRenderer_t g_OldSetupRenderer = nullptr;

void MySetupRenderer(int rendermode)
{
	g_OldSetupRenderer(rendermode);
	g_GameRecord.RecordCurrentEntity();
}

bool Hook_R_SetupRenderer()
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

		g_OldSetupRenderer = pstudio->SetupRenderer;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)g_OldSetupRenderer, MySetupRenderer);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\nHook_R_SetupRenderer");
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
		&& Hook_R_SetupRenderer()
		&& Hook_R_SetRenderModel()
		&& Hook_R_StudioSetHeader()
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

	if (!m_AfxGameRecord.StartRecording(strFileName.c_str(), 6))
	{
		pEngfuncs->Con_Printf("AfxError: could not start recording to \"%s\".\n", fileName);
		return false;
	}

	return true;
}

void CGameRecord::EndRecording()
{
	if (!GetRecording()) return;

	m_AfxGameRecord.EndRecording();

	m_Indexes.clear();
}

void CGameRecord::OnFrameBegin()
{
	if (!GetRecording()) return;

	m_Header = nullptr;

	m_AfxGameRecord.BeginFrame((float)*g_phost_frametime);
}

void CGameRecord::BeforeHostFrame()
{
	if (!GetRecording()) return;

	std::set<cl_entity_t*> ents;

	for (int i = 0; i < 2048; ++i)
	{
		cl_entity_t* pEnt = pEngfuncs->GetEntityByIndex(i);

		if (pEnt && pEnt->model) {
			ents.emplace(pEnt);
		}

	}

	for (auto it = m_Indexes.begin(); it != m_Indexes.end(); )
	{
		if (ents.find(it->first) == ents.end())
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				m_AfxGameRecord.MarkHidden(it2->second);
			}

			it = m_Indexes.erase(it);
		}
		else ++it;
	}
}

void CGameRecord::OnFrameEnd(float view_origin[3], float view_angles[3], float fov)
{
	if (!GetRecording()) return;

	struct engine_studio_api_s* pstudio = (struct engine_studio_api_s*)HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio);

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
	if (!GetRecording()) return;

	struct engine_studio_api_s* pstudio = (struct engine_studio_api_s*)HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio);

	if (m_Model == nullptr) return;
	if (m_Header == nullptr) return;


	if (cl_entity_s* ent = pstudio->GetCurrentEntity())
	{
		if (ent->curstate.weaponmodel)
		{
			if (struct model_s * pweaponmodel = pstudio->GetModelByIndex(ent->curstate.weaponmodel))
			{
				if(studiohdr_t * weaponmodelheader = (studiohdr_t*)pstudio->Mod_Extradata(pweaponmodel))
				{
					if (weaponmodelheader == m_Header)
					{
						// It's doing the weapon submodel instead.
						RecordModel(ent, pweaponmodel, m_Header);
						return;
					}
				}
			}
		}

		RecordModel(ent, m_Model, m_Header);
	}
}

bool InvertMatrix(const float matrix[3][4], float out_matrix[3][4]) {

	double b0[4] = { 1, 0, 0, 0 };
	double b1[4] = { 0, 1, 0, 0 };
	double b2[4] = { 0, 0, 1, 0 };
	double b3[4] = { 0, 0, 0, 1 };

	double M[4][4] = {
		matrix[0][0], matrix[0][1], matrix[0][2],  matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2],  matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2],  matrix[2][3],
		0, 0, 0, 1
	};

	unsigned char P[4];
	unsigned char Q[4];

	double L[4][4];
	double U[4][4];

	if (!Afx::Math::LUdecomposition(M, P, Q, L, U))
		return false;

	double inv0[4] = { 1,0,0,0 };
	double inv1[4] = { 0,1,0,0 };
	double inv2[4] = { 0,0,1,0 };
	double inv3[4] = { 0,0,0,1 };

	Afx::Math::SolveWithLU(L, U, P, Q, b0, inv0);
	Afx::Math::SolveWithLU(L, U, P, Q, b1, inv1);
	Afx::Math::SolveWithLU(L, U, P, Q, b2, inv2);
	Afx::Math::SolveWithLU(L, U, P, Q, b3, inv3);

	out_matrix[0][0] = (float)inv0[0];
	out_matrix[0][1] = (float)inv1[0];
	out_matrix[0][2] = (float)inv2[0];
	out_matrix[0][3] = (float)inv3[0];

	out_matrix[1][0] = (float)inv0[1];
	out_matrix[1][1] = (float)inv1[1];
	out_matrix[1][2] = (float)inv2[1];
	out_matrix[1][3] = (float)inv3[1];

	out_matrix[2][0] = (float)inv0[2];
	out_matrix[2][1] = (float)inv1[2];
	out_matrix[2][2] = (float)inv2[2];
	out_matrix[2][3] = (float)inv3[2];

	//pEngfuncs->Con_Printf("(%f, %f, %f, %f)\n", inv0[3], inv1[3], inv2[3], inv3[3]);

	return true;
}

void ToVecQuat(float bones[3][4], float out_vec[3], float out_quat[4]) {

	out_vec[0] = bones[0][3];
	out_vec[1] = bones[1][3];
	out_vec[2] = bones[2][3];

	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm

	float qw = 1.0f;
	float qx = 0.0f;
	float qy = 0.0f;
	float qz = 0.0f;

	float tr = bones[0][0] + bones[1][1] + bones[2][2];

	if (tr > 0) { 
	float S = sqrt(tr+1.0) * 2; // S=4*qw 
	qw = 0.25 * S;
	qx = (bones[2][1] - bones[1][2]) / S;
	qy = (bones[0][2] - bones[2][0]) / S; 
	qz = (bones[1][0] - bones[0][1]) / S; 
	} else if ((bones[0][0] > bones[1][1])&&(bones[0][0] > bones[2][2])) { 
	float S = sqrt(1.0 + bones[0][0] - bones[1][1] - bones[2][2]) * 2; // S=4*qx 
	qw = (bones[2][1] - bones[1][2]) / S;
	qx = 0.25 * S;
	qy = (bones[0][1] + bones[1][0]) / S; 
	qz = (bones[0][2] + bones[2][0]) / S; 
	} else if (bones[1][1] > bones[2][2]) { 
	float S = sqrt(1.0 + bones[1][1] - bones[0][0] - bones[2][2]) * 2; // S=4*qy
	qw = (bones[0][2] - bones[2][0]) / S;
	qx = (bones[0][1] + bones[1][0]) / S; 
	qy = 0.25 * S;
	qz = (bones[1][2] + bones[2][1]) / S; 
	} else { 
	float S = sqrt(1.0 + bones[2][2] - bones[0][0] - bones[1][1]) * 2; // S=4*qz
	qw = (bones[1][0] - bones[0][1]) / S;
	qx = (bones[0][2] + bones[2][0]) / S;
	qy = (bones[1][2] + bones[2][1]) / S;
	qz = 0.25 * S;
	}

	out_quat[0] = qx;
	out_quat[1] = qy;
	out_quat[2] = qz;
	out_quat[3] = qw;
}

void CGameRecord::RecordModel(cl_entity_t* ent, struct model_s* model, void* v_header)
{
	struct engine_studio_api_s* pstudio = (struct engine_studio_api_s*)HL_ADDR_GET(hw_HUD_GetStudioModelInterface_pStudio);

	int index = -1;
	auto emplaced = m_Indexes.emplace(std::piecewise_construct, std::forward_as_tuple(ent), std::forward_as_tuple());
	if (emplaced.second)
	{
		m_Index++;
		index = m_Index;
		emplaced.first->second.emplace(v_header, index);
	}
	else {
		auto emplaced2 = emplaced.first->second.emplace(v_header, m_Index + 1);
		if (emplaced2.second)
		{
			m_Index++;
			index = m_Index;
		}
		else index = emplaced2.first->second;
	}

	studiohdr_t* header = (studiohdr_t*)v_header;

	{
		int numbones = header->numbones;

		mstudiobone_t* pbones = (mstudiobone_t*)((byte*)header + header->boneindex);

		float(*pBoneM)[MAXSTUDIOBONES][3][4] = (float(*)[MAXSTUDIOBONES][3][4])(pstudio->StudioGetBoneTransform());
		float(*pRotM)[3][4] = (float(*)[3][4])(pstudio->StudioGetRotationMatrix());

		if (0 < numbones && pBoneM && pRotM)
		{
			float bones[3][4];
			float tmp[3][4];
			float rootMatrix[3][4];

			m_AfxGameRecord.WriteDictionary("entity_state");
			m_AfxGameRecord.Write((int)index);

			m_AfxGameRecord.WriteDictionary("baseentity");
			//Write((float)pBaseEntityRs->m_flTime);
			m_AfxGameRecord.WriteDictionary(model->name);
			m_AfxGameRecord.Write((bool)true);

			WriteMatrix3x4(*pRotM);

			m_AfxGameRecord.WriteDictionary("baseanimating");
			//Write((int)pBaseAnimatingRs->m_nSkin);
			//Write((int)pBaseAnimatingRs->m_nBody);
			//Write((int)pBaseAnimatingRs->m_nSequence);
			m_AfxGameRecord.Write((bool)true);

			m_AfxGameRecord.Write((int)header->numbones);


			for (int i = 0; i < header->numbones; ++i)
			{
				bool bRoot = pbones[i].parent == -1;

				if (!InvertMatrix(bRoot ? (*pRotM) : (*pBoneM)[pbones[i].parent], tmp))
					pEngfuncs->Con_Printf("AFXERROR: parent matrix inversion failed for bone %i (model \"%s\").\n", i, model->name);

				R_ConcatTransforms(tmp, (*pBoneM)[i], bones);

				WriteMatrix3x4(bones);
			}

			m_AfxGameRecord.WriteDictionary("/");

			bool viewModel = false;

			m_AfxGameRecord.Write((bool)viewModel);
		}
	}
}

void CGameRecord::StudioSetHeader(void* header)
{
	m_Header = header;
}

void CGameRecord::SetRenderModel(struct model_s* model) {
	m_Model = model;
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

void CGameRecord::WriteMatrix3x4(float matrix[3][4])
{
	m_AfxGameRecord.Write(matrix[0][0]);
	m_AfxGameRecord.Write(matrix[0][1]);
	m_AfxGameRecord.Write(matrix[0][2]);
	m_AfxGameRecord.Write(matrix[0][3]);

	m_AfxGameRecord.Write(matrix[1][0]);
	m_AfxGameRecord.Write(matrix[1][1]);
	m_AfxGameRecord.Write(matrix[1][2]);
	m_AfxGameRecord.Write(matrix[1][3]);

	m_AfxGameRecord.Write(matrix[2][0]);
	m_AfxGameRecord.Write(matrix[2][1]);
	m_AfxGameRecord.Write(matrix[2][2]);
	m_AfxGameRecord.Write(matrix[2][3]);
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
		"%s start <sFilePath> - Start recording to file <sFilePath>, you should set a low FPS before (i.e. using host_framerate 0.04 (1/25 FPS = 0.04 seconds per frame)) and give the \".agr\" file extension.\n"
		"%s stop - Stop recording.\n"
		, prefix
		, prefix
	);

}
#pragma once

// 0471e320

enum ClientFunctionTableEntry
{
	CFTE_Initialize = 0,
	CFTE_HUD_Init,
	CFTE_HUD_VidInit,
	CFTE_HUD_Redraw,
	CFTE_HUD_UpdateClientData,
	CFTE_HUD_Reset,
	CFTE_HUD_PlayerMove,
	CFTE_HUD_PlayerMoveInit,
	CFTE_HUD_PlayerMoveTexture,
	CFTE_IN_ActivateMouse,
	CFTE_IN_DeactivateMouse,
	CFTE_IN_MouseEvent,
	CFTE_IN_ClearStates,
	CFTE_IN_Accumulate,
	CFTE_CL_CreateMove,
	CFTE_CL_IsThirdPerson,
	CFTE_CL_CameraOffset,
	CFTE_KB_Find,
	CFTE_CAM_Think,
	CFTE_V_CalcRefdef,
	CFTE_HUD_AddEntity,
	CFTE_HUD_CreateEntities,
	CFTE_HUD_DrawNormalTriangles,
	CFTE_HUD_DrawTransparentTriangles,
	CFTE_HUD_StudioEvent,
	CFTE_HUD_PostRunCmd,
	CFTE_HUD_Shutdown,
	CFTE_HUD_TxferLocalOverrides,
	CFTE_HUD_ProcessPlayerState,
	CFTE_HUD_TxferPredictionData,
	CFTE_Demo_ReadBuffer,
	CFTE_HUD_ConnectionlessPacket,
	CFTE_HUD_GetHullBounds,
	CFTE_HUD_Frame,
	CFTE_HUD_Key_Event,
	CFTE_HUD_TempEntUpdate,
	CFTE_HUD_GetUserEntity,
	CFTE_HUD_VoiceStatus,
	CFTE_HUD_DirectorMessage,
	CFTE_HUD_GetStudioModelInterface,
	CFTE_HUD_ChatInputPosition,
	CFTE_HUD_GetPlayerTeam, // unused it seems, not sure if it works
	CFTE_ClientFactory
};

void HookClientFunctions();

void * GetClientFunction(ClientFunctionTableEntry entry);

void ReplaceClientFunction(ClientFunctionTableEntry entry, void * newFunction);

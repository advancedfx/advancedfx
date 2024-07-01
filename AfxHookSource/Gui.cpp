#include "stdafx.h"

// TODO:
// - Lots.
// - Maybe handle RawInput mouse buttons in drag mode (so the game doesn't see them, but currently it doesn't care about them anyway.).

#include "Gui.h"

#ifndef AFX_ENABLE_GUI
#define AFX_ENABLE_GUI 0
#endif

#if AFX_ENABLE_GUI

#include <shared/imgui/imgui.h>

#include <SourceInterfaces.h>
#include "WrpVEngineClient.h"
#include "addresses.h"
#include "hlaeFolder.h"
#include <list>
#include <string>
#include <vector>
#include <mutex>

#include "Model/Schedule.h"

extern SourceSdkVer g_SourceSdkVer;
extern WrpVEngineClient * g_VEngineClient;

namespace AfxHookSource {
namespace Gui {

bool m_Active = true;
bool m_KeyDownEaten[512] = {};
bool m_PassThrough = false;
POINT m_GameCursorPos = { 0, 0 };
POINT m_OldCursorPos = { 0, 0 };
HCURSOR m_OwnCursor = 0;
HCURSOR m_GameCursor = 0;
bool m_CursorSet = false;
std::string m_IniFilename;
std::string m_LogFileName;
bool m_FirstEndScene = true;
bool m_GameCaptured = false;
bool m_InMouseLook = false;
bool m_HadSetCursorMouseLook = false;

//Model::CSchedule m_Schedule(nullptr, "Schedule", "");

// Data
static HWND                     g_hWnd = 0;
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;

static class Cursors_s
{
private:
	HCURSOR m_None = 0;
	HCURSOR m_Arrow = LoadCursor(NULL, IDC_ARROW);
	HCURSOR m_TextInput = LoadCursor(NULL, IDC_IBEAM);
	HCURSOR m_ResizeAll = LoadCursor(NULL, IDC_SIZEALL);
	HCURSOR m_ResizeNS = LoadCursor(NULL, IDC_SIZENS);
	HCURSOR m_ResizeEW = LoadCursor(NULL, IDC_SIZEWE);
	HCURSOR m_ResizeNESW = LoadCursor(NULL, IDC_SIZENESW);
	HCURSOR m_ResizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);

	HCURSOR m_Hand = LoadCursor(NULL, IDC_HAND);

public:
	HCURSOR GetCursor(ImGuiMouseCursor imGuiMouseCursor)
	{
		switch (imGuiMouseCursor)
		{
		case ImGuiMouseCursor_None:
			return m_None;
		case ImGuiMouseCursor_Arrow:
			return m_Arrow;
		case ImGuiMouseCursor_TextInput:
			return m_TextInput;
		case ImGuiMouseCursor_ResizeAll:
			return m_ResizeAll;
		case ImGuiMouseCursor_ResizeNS:
			return m_ResizeNS;
		case ImGuiMouseCursor_ResizeEW:
			return m_ResizeEW;
		case ImGuiMouseCursor_ResizeNESW:
			return m_ResizeNESW;
		case ImGuiMouseCursor_ResizeNWSE:
			return m_ResizeNWSE;
		}

		return m_Arrow;
	}

	HCURSOR GetFromGame(HCURSOR gameCursor, bool inMouseLook)
	{
		if (!inMouseLook || gameCursor)
			return gameCursor;
		return m_Hand;
	}

} m_Cursors;

struct CUSTOMVERTEX
{
	float    pos[3];
	D3DCOLOR col;
	float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

bool IsSupported()
{
	//return false;
	return SourceSdkVer_CSGO == g_SourceSdkVer || SourceSdkVer_CSCO == g_SourceSdkVer;
}

bool IsInMouseLook()
{
	return m_InMouseLook;
}

class CImGuiRenderCacheManager
{
public:
	void Dx9_Create(IDirect3DDevice9 * device)
	{
		Dx9_Destroy();

		m_Device = device;

		std::unique_lock<std::mutex> freeLock(m_FreeCachesMutex);

		for (std::list<CImGuiRenderCache *>::iterator it = m_FreeCaches.begin(); it != m_FreeCaches.end(); ++it)
		{
			(*it)->Dx9_Create(device);
		}
	}

	void Dx9_Destroy()
	{
		std::unique_lock<std::mutex> freeLock(m_FreeCachesMutex);
		std::unique_lock<std::mutex> queuedLock(m_QueuedCachesMutex);

		while (!m_QueuedCaches.empty())
		{
			m_FreeCaches.push_back(m_QueuedCaches.front());
			m_QueuedCaches.pop_front();
		}

		for (std::list<CImGuiRenderCache *>::iterator it = m_FreeCaches.begin(); it != m_FreeCaches.end(); ++it)
		{
			(*it)->Dx9_Destroy();
		}

		m_Device = 0;
	}

	void Dx9_Render()
	{
		CImGuiRenderCache * cache = 0;

		{
			std::unique_lock<std::mutex> queuedLock(m_QueuedCachesMutex);

			if (0 < m_QueuedCaches.size())
			{
				cache = m_QueuedCaches.front();

				m_QueuedCaches.pop_front();
			}
		}

		if (cache)
		{
			cache->Render();

			{
				std::unique_lock<std::mutex> freeLock(m_FreeCachesMutex);

				m_FreeCaches.push_back(cache);
			}
		}
	}

	void CacheRender(ImDrawData* draw_data)
	{
		CImGuiRenderCache * cache = 0;

		{
			std::unique_lock<std::mutex> freeLock(m_FreeCachesMutex);

			if (0 < m_FreeCaches.size())
			{
				cache = m_FreeCaches.front();
				m_FreeCaches.pop_front();
			}
		}

		if (!cache)
		{
			cache = new CImGuiRenderCache();
			if (m_Device) cache->Dx9_Create(m_Device);
		}

		cache->CacheRender(draw_data);

		{
			std::unique_lock<std::mutex> queuedLock(m_QueuedCachesMutex);

			m_QueuedCaches.push_back(cache);
		}
	}

private:
	class CImGuiRenderCache
	{
	public:
		void Dx9_Create(IDirect3DDevice9 * device)
		{
			Dx9_Destroy();

			m_Device = device;
		}

		void Dx9_Destroy()
		{
			if (m_pIB)
			{
				m_pIB->Release();
				m_pIB = 0;
			}

			if (m_pVB)
			{
				m_pVB->Release();
				m_pVB = 0;
			}

			if (m_Device)
			{
				m_Device = 0;
			}
		}

		void CacheRender(ImDrawData* draw_data)
		{
			m_DrawListCount = 0;

			if (!m_Device)
				return;

			ImGuiIO& io = ImGui::GetIO();

			// Avoid rendering when minimized
			if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
				return;

			m_Width = io.DisplaySize.x;
			m_Height = io.DisplaySize.y;

			if (m_DrawListSize < draw_data->CmdListsCount)
			{
				m_DrawLists.resize(draw_data->CmdListsCount);
				m_DrawListSize = draw_data->CmdListsCount;
			}

			// Create and grow buffers if needed
			if (!m_pVB || m_VertexBufferSize < draw_data->TotalVtxCount)
			{
				if (m_pVB) { m_pVB->Release(); m_pVB = NULL; }
				m_VertexBufferSize = draw_data->TotalVtxCount + 5000;
				if (m_Device->CreateVertexBuffer(m_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVB, NULL) < 0)
					return;
			}
			if (!m_pIB || m_IndexBufferSize < draw_data->TotalIdxCount)
			{
				if (m_pIB) { m_pIB->Release(); m_pIB = NULL; }
				m_IndexBufferSize = draw_data->TotalIdxCount + 10000;
				if (m_Device->CreateIndexBuffer(m_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIB, NULL) < 0)
					return;
			}

			// Copy and convert all vertices into a single contiguous buffer
			CUSTOMVERTEX* vtx_dst;
			ImDrawIdx* idx_dst;

			if (m_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
				return;
			if (m_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
				return;

			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
				for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
				{
					vtx_dst->pos[0] = vtx_src->pos.x;
					vtx_dst->pos[1] = vtx_src->pos.y;
					vtx_dst->pos[2] = 0.0f;
					vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
					vtx_dst->uv[0] = vtx_src->uv.x;
					vtx_dst->uv[1] = vtx_src->uv.y;
					vtx_dst++;
					vtx_src++;
				}
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			m_pVB->Unlock();
			m_pIB->Unlock();

			// Copy command list:
			//

			m_DrawListCount = draw_data->CmdListsCount;

			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				CDrawList & drawList = m_DrawLists[n];

				const ImDrawList* cmd_list = draw_data->CmdLists[n];

				if (drawList.m_DrawCommandSize < cmd_list->CmdBuffer.Size)
				{
					drawList.m_DrawCommands.resize(cmd_list->CmdBuffer.Size);
					drawList.m_DrawCommandSize = cmd_list->CmdBuffer.Size;
				}

				drawList.m_DrawCommandCount = cmd_list->CmdBuffer.Size;

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					drawList.m_DrawCommands[cmd_i] = cmd_list->CmdBuffer[cmd_i];
				}

				drawList.m_VtxBuffer_Size = cmd_list->VtxBuffer.Size;
			}

		}

		void Render(void)
		{
			if (!m_Device)
				return;

			if (0 == m_DrawListCount)
				return;

			// Backup the DX9 state
			IDirect3DStateBlock9* d3d9_state_block = NULL;
			if (m_Device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
				return;

			m_Device->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
			m_Device->SetIndices(m_pIB);
			m_Device->SetFVF(D3DFVF_CUSTOMVERTEX);

			// Setup viewport
			D3DVIEWPORT9 vp;
			vp.X = vp.Y = 0;
			vp.Width = (DWORD)m_Width;
			vp.Height = (DWORD)m_Height;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			m_Device->SetViewport(&vp);

			// Additional state (added):
			m_Device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
			m_Device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
			m_Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);

			// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
			m_Device->SetPixelShader(NULL);
			m_Device->SetVertexShader(NULL);
			m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
			m_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
			m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			m_Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			m_Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			m_Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


			// Setup orthographic projection matrix
			// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
			{
				const float L = 0.5f, R = m_Width + 0.5f, T = 0.5f, B = m_Height + 0.5f;
				D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
				D3DMATRIX mat_projection =
				{
					2.0f / (R - L),   0.0f,         0.0f,  0.0f,
					0.0f,         2.0f / (T - B),   0.0f,  0.0f,
					0.0f,         0.0f,         0.5f,  0.0f,
					(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f,
				};
				m_Device->SetTransform(D3DTS_WORLD, &mat_identity);
				m_Device->SetTransform(D3DTS_VIEW, &mat_identity);
				m_Device->SetTransform(D3DTS_PROJECTION, &mat_projection);
			}

			// Render command lists
			int vtx_offset = 0;
			int idx_offset = 0;
			for (int n = 0; n < m_DrawListCount; n++)
			{
				const CDrawList & cmd_list = m_DrawLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list.m_DrawCommandCount; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list.m_DrawCommands[cmd_i];
					if (pcmd->UserCallback)
					{
						pcmd->UserCallback(/* cmd_list */ NULL, pcmd);
					}
					else
					{
						const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
						m_Device->SetTexture(0, pcmd->TextureId && pcmd->TextureId == (void *)1 ? g_FontTexture : NULL);
						m_Device->SetScissorRect(&r);
						m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list.m_VtxBuffer_Size, idx_offset, pcmd->ElemCount / 3);
					}
					idx_offset += pcmd->ElemCount;
				}
				vtx_offset += cmd_list.m_VtxBuffer_Size;
			}

			// Restore the DX9 state
			d3d9_state_block->Apply();
			d3d9_state_block->Release();
		}

	private:
		IDirect3DDevice9 * m_Device = 0;
		LPDIRECT3DVERTEXBUFFER9 m_pVB = 0;
		LPDIRECT3DINDEXBUFFER9 m_pIB = 0;
		int m_VertexBufferSize = 5000;
		int m_IndexBufferSize = 10000;

		struct CDrawList
		{
			int m_DrawCommandCount;
			int m_DrawCommandSize = 0;
			std::vector<ImDrawCmd> m_DrawCommands;
			int m_VtxBuffer_Size;
		};

		int m_DrawListCount;
		int m_DrawListSize = 0;
		std::vector<CDrawList> m_DrawLists;
		float m_Width;
		float m_Height;

	};

	std::mutex m_FreeCachesMutex;
	std::list<CImGuiRenderCache *> m_FreeCaches;
	std::mutex m_QueuedCachesMutex;
	std::list<CImGuiRenderCache *> m_QueuedCaches;
	IDirect3DDevice9 * m_Device = 0;
} m_ImGuiRernderCacheManager;


void CacheRenderDrawData(ImDrawData* data)
{
	m_ImGuiRernderCacheManager.CacheRender(data);
}


static bool IsAnyMouseButtonDown()
{
	ImGuiIO& io = ImGui::GetIO();
	for (int n = 0; n < ARRAYSIZE(io.MouseDown); n++)
		if (io.MouseDown[n])
			return true;
	return false;
}

enum EndPassThrougButtonUp
{
	EPTBU_Left,
	EPTBU_Right,
	EPTBU_Middle
};
EndPassThrougButtonUp m_EndPassThroughButtonUp = EPTBU_Left;
bool m_IgnoreNextWM_MOUSEMOVE = false;

bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam)
{
	if (!IsSupported())
		return false;

	if (!m_Active)
		return false;

	if (m_PassThrough)
	{
		bool end = false;

		switch (msg)
		{
		case WM_LBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Left) end = true;
			break;
		case WM_RBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Right) end = true;
			break;
		case WM_MBUTTONUP:
			if (m_EndPassThroughButtonUp == EPTBU_Middle) end = true;
			break;
		case WM_MOUSEMOVE:
			switch (m_EndPassThroughButtonUp)
			{
			case EPTBU_Left:
				wParam &= ~(WPARAM)MK_LBUTTON;
				break;
			case EPTBU_Right:
				wParam &= ~(WPARAM)MK_RBUTTON;
				break;
			case EPTBU_Middle:
				wParam &= ~(WPARAM)MK_MBUTTON;
				break;
			}
			break;
		}

		if (end)
		{
			GetCursorPos(&m_GameCursorPos);

			m_PassThrough = false;

			SetCursor(m_OwnCursor);
			SetCursorPos(m_OldCursorPos.x, m_OldCursorPos.y);

			if (m_GameCaptured && GetCapture() == hwnd)
				ReleaseCapture();

			return true;
		}

		return false;
	}

	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		return io.WantCaptureMouse;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			if (!io.WantCaptureMouse && IsInMouseLook())
			{
				bool start = false;

				switch (msg)
				{
				case WM_LBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Left;
					break;
				case WM_RBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Right;
					break;
				case WM_MBUTTONDOWN:
					start = true;
					m_EndPassThroughButtonUp = EPTBU_Middle;
					break;
				}
				
				if (start)
				{
					GetCursorPos(&m_OldCursorPos);

					m_IgnoreNextWM_MOUSEMOVE = m_OldCursorPos.x != m_GameCursorPos.x || m_OldCursorPos.y != m_GameCursorPos.y;

					SetCursor(m_GameCursor);
					SetCursorPos(m_GameCursorPos.x, m_GameCursorPos.y);

					if (m_GameCaptured && GetCapture() == NULL)
						SetCapture(hwnd);

					m_PassThrough = true;
					return true;
				}
			}
		
			int button = 0;
			if (msg == WM_LBUTTONDOWN) button = 0;
			if (msg == WM_RBUTTONDOWN) button = 1;
			if (msg == WM_MBUTTONDOWN) button = 2;
			if (!IsAnyMouseButtonDown() && GetCapture() == NULL)
				SetCapture(hwnd);
			io.MouseDown[button] = true;
			return io.WantCaptureMouse || IsInMouseLook();
		}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			int button = 0;
			if (msg == WM_LBUTTONUP) button = 0;
			if (msg == WM_RBUTTONUP) button = 1;
			if (msg == WM_MBUTTONUP) button = 2;
			io.MouseDown[button] = false;
			if (!IsAnyMouseButtonDown() && GetCapture() == hwnd)
				ReleaseCapture();
			return io.WantCaptureMouse || IsInMouseLook();
		}
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return io.WantCaptureMouse || IsInMouseLook();
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return io.WantCaptureMouse || IsInMouseLook();
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
		{
			io.KeysDown[wParam] = 1;
			m_KeyDownEaten[wParam] = io.WantCaptureKeyboard;
		}
		return io.WantCaptureKeyboard;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			bool wasKeyDownEaten = false;
			if (wParam < 256)
			{
				io.KeysDown[wParam] = 0;
				wasKeyDownEaten = m_KeyDownEaten[wParam];
				m_KeyDownEaten[wParam] = false;
			}
			return io.WantCaptureKeyboard || wasKeyDownEaten;
		}
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return io.WantCaptureKeyboard;
	case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			switch (inp.header.dwType)
			{
			case RIM_TYPEMOUSE:
				return IsInMouseLook();
			case RIM_TYPEKEYBOARD:
				return io.WantCaptureKeyboard;
			}
			return false;
		}
	}
	return false;
}

bool OnSetCursorPos(__in int X, __in int Y)
{
	if (!IsSupported())
		return false;

	if (g_hWnd && m_GameCaptured && g_VEngineClient)
	{
		int width, height;
		POINT clientPoint = { X , Y };

		g_VEngineClient->GetScreenSize(width, height);

		ScreenToClient(g_hWnd, &clientPoint);

		m_HadSetCursorMouseLook = ((width >> 1) == clientPoint.x) && ((height >> 1) == clientPoint.y);

		//Tier0_Msg("%i == %i, %i == %i + (%i, %i) --> %i\n", width >> 1, clientPoint.x, height >> 1, clientPoint.y, X, Y, m_InMouseLook ? 1 : 0);
	}
	else
		m_HadSetCursorMouseLook = false;

	m_GameCursorPos.x = X;
	m_GameCursorPos.y = Y;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!IsInMouseLook())
		return false;

	return true;
}

bool OnGetCursorPos(__out LPPOINT lpPoint)
{
	if (!IsSupported())
		return false;

	if (!m_Active || m_PassThrough || !IsInMouseLook())
	{
		if (lpPoint)
		{
			m_GameCursorPos.x = lpPoint->x;
			m_GameCursorPos.y = lpPoint->y;
		}

		return false;
	}

	if (lpPoint)
	{
		lpPoint->x = m_GameCursorPos.x;
		lpPoint->y = m_GameCursorPos.y;
	}

	return true;
}

bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result)
{
	if (!IsSupported())
		return false;

	if (!m_CursorSet)
	{
		m_GameCursor = GetCursor();
		m_CursorSet = true;
	}

	HCURSOR oldCursor = m_GameCursor;

	m_GameCursor = hCursor;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		m_OwnCursor = m_Cursors.GetFromGame(hCursor, IsInMouseLook());
	}

	result = oldCursor;
	SetCursor(m_OwnCursor);

	return true;
}

bool OnSetCapture(HWND hWnd, HWND & result)
{
	if (!IsSupported())
		return false;

	m_GameCaptured = hWnd && hWnd == g_hWnd;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!IsInMouseLook())
		return false;

	result = 0; // TODO: maybe some smartass logic here to determine something (not really worth it probably atm)?
	return true;
}

bool OnReleaseCapture()
{
	if (!IsSupported())
		return false;

	bool wasInMouseLook = IsInMouseLook();

	m_GameCaptured = false;
	m_InMouseLook = false;

	if (!m_Active)
		return false;

	if (m_PassThrough)
		return false;

	if (!wasInMouseLook)
		return false;

	return true;
}


bool DX9_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height, bytes_per_pixel;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

	// Upload texture to graphics system
	g_FontTexture = NULL;
	if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, NULL) < 0)
		return false;
	D3DLOCKED_RECT tex_locked_rect;
	if (g_FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
		return false;
	for (int y = 0; y < height; y++)
		memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
	g_FontTexture->UnlockRect(0);

	io.Fonts->TexID = (void *)1;

	return true;
}

bool DX9_CreateDeviceObjects()
{
	if (!g_pd3dDevice)
		return false;
	if (!DX9_CreateFontsTexture())
		return false;

	m_ImGuiRernderCacheManager.Dx9_Create(g_pd3dDevice);

	return true;
}

void DllProcessAttach(void)
{
	ImGui::SetCurrentContext(ImGui::CreateContext());
}

void DllProcessDetach(void)
{
	ImGui::DestroyContext(ImGui::GetCurrentContext());
}

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device)
{
	if (!IsSupported())
		return false;

	g_hWnd = (HWND)hwnd;
	g_pd3dDevice = device;

	if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
		return false;
	if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
		return false;

	ImGuiIO& io = ImGui::GetIO();

	std::string hlaeFolderPath(GetHlaeFolder());
	
	m_IniFilename = hlaeFolderPath;
	m_IniFilename.append("\\AfxHookSource_imgui.ini");

	io.IniFilename = m_IniFilename.c_str();

	m_LogFileName = hlaeFolderPath;
	m_LogFileName.append("\\AfxHookmource_imgui_log.txt");
	io.LogFilename = m_LogFileName.c_str();

	io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.ImeWindowHandle = g_hWnd;

	DX9_CreateDeviceObjects();

	return true;
}

void DX9_InvalidateDeviceObjects()
{
	if (!g_pd3dDevice)
		return;

	m_ImGuiRernderCacheManager.Dx9_Destroy();

	// At this point note that we set ImGui::GetIO().Fonts->TexID to be == g_FontTexture, so clear both.
	ImGuiIO& io = ImGui::GetIO();
	if (g_FontTexture)
		g_FontTexture->Release();
	g_FontTexture = NULL;
	io.Fonts->TexID = NULL;
}

void DoFrame()
{
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Demo Window")) show_demo_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Look: %i | Want: %i | Through: %i\n", IsInMouseLook() ? 1 : 0, ImGui::GetIO().WantCaptureMouse ? 1 : 0, m_PassThrough ? 1 : 0);
	}

	// 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name the window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow().
	if (show_demo_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&show_demo_window);
	}
}

void On_Direct3DDevice9_Shutdown()
{
	if (!IsSupported())
		return;

	DX9_InvalidateDeviceObjects();
	g_pd3dDevice = NULL;
	g_hWnd = 0;
}


void On_Direct3DDevice9_EndScene()
{
	if (!IsSupported())
		return;

	if (m_FirstEndScene && m_Active && g_FontTexture)
	{
		m_FirstEndScene = false;

		m_ImGuiRernderCacheManager.Dx9_Render();
	}
}

void On_Direct3DDevice9_Present(bool deviceLost)
{
	if (!IsSupported())
		return;

	m_FirstEndScene = true;
}

void On_Direct3DDevice9_Reset_Before()
{
	if (!IsSupported())
		return;

	DX9_InvalidateDeviceObjects();
}

void On_Direct3DDevice9_Reset_After()
{
	if (!IsSupported())
		return;

	DX9_CreateDeviceObjects();
}


bool OnGameFrameRenderEnd()
{
	if (!IsSupported())
		return false;

	if (!m_Active)
		return false;

	ImGuiIO& io = ImGui::GetIO();

	// Setup time step
	INT64 current_time;
	QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
	io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
	g_Time = current_time;

	// Setup display size (every frame to accommodate for window resizing)
	RECT rect;
	GetClientRect(g_hWnd, &rect);
	io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

	// Read keyboard modifiers inputs
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;
	// io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
	// io.MousePos : filled by WM_MOUSEMOVE events
	// io.MouseDown : filled by WM_*BUTTON* events
	// io.MouseWheel : filled by WM_MOUSEWHEEL events

	// Set OS mouse position if requested last frame by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
	if (io.WantMoveMouse)
	{
		POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
		ClientToScreen(g_hWnd, &pos);
		SetCursorPos(pos.x, pos.y);
	}

	//// Hide OS mouse cursor if ImGui is drawing it
	//if (io.MouseDrawCursor)
	//	SetCursor(NULL);

	HCURSOR cursor = 0;

	if (io.WantCaptureMouse)
	{
		cursor = m_Cursors.GetCursor(ImGui::GetMouseCursor());
	}
	else
	{
		cursor = m_Cursors.GetFromGame(m_GameCursor, IsInMouseLook());
	}

	if (m_OwnCursor != cursor)
	{
		m_OwnCursor = cursor;
		SetCursor(m_OwnCursor);
	}

	m_InMouseLook = m_HadSetCursorMouseLook;
	m_HadSetCursorMouseLook = false;

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();

	DoFrame();

	ImGui::EndFrame();

	ImGui::Render();

	CacheRenderDrawData(ImGui::GetDrawData());

	return true;
}

} // namespace Gui {
} // namespace AfxHookSource {

#else

#include "Gui.h"

namespace AfxHookSource {
namespace Gui {

void DllProcessAttach(void)
{
}

void DllProcessDetach(void)
{
}

bool WndProcHandler(HWND hwnd, UINT msg, WPARAM & wParam, LPARAM & lParam)
{
	return false;
}

bool OnSetCursorPos(__in int X, __in int Y)
{
	return false;
}

bool OnGetCursorPos(__out LPPOINT lpPoint)
{
	return false;
}

bool OnSetCursor(__in_opt HCURSOR hCursor, HCURSOR & result)
{
	return false;
}

bool OnSetCapture(HWND hWnd, HWND & result)
{
	return false;
}

bool OnReleaseCapture()
{
	return false;
}

bool OnGameFrameRenderEnd()
{
	return false;
}

bool On_Direct3DDevice9_Init(void* hwnd, IDirect3DDevice9* device)
{
	return false;
}

void On_Direct3DDevice9_Shutdown()
{

}

void On_Direct3DDevice9_EndScene()
{

}

void On_Direct3DDevice9_Present(bool deviceLost)
{

}

void On_Direct3DDevice9_Reset_Before()
{

}

void On_Direct3DDevice9_Reset_After()
{

}



} // namespace Gui {
} // namespace AfxHookSource {


#endif

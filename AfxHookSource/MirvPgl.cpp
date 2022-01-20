#include "stdafx.h"

#ifdef AFX_MIRV_PGL
// Needs to be included for d3d9.h or we a doomed (great!):
#include <deps/release/easywsclient/easywsclient.hpp>
#include <deps/release/easywsclient/easywsclient.cpp>
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

#include "MirvPgl.h"

#ifdef AFX_MIRV_PGL

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "AfxStreams.h"
#include "AfxShaders.h"
#include "csgo_GameEvents.h"
#include "FovScaling.h"
#include "RenderView.h"

#include <shared/AfxMath.h>

#include <math.h>

#include <string>
#include <list>
#include <queue>
#include <set>

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>

extern WrpVEngineClient * g_VEngineClient;


using namespace std::chrono_literals;

using easywsclient::WebSocket;

namespace MirvPgl
{
	const D3DVERTEXELEMENT9 g_Drawing_VBDecl_Position[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	const D3DVERTEXELEMENT9 g_Drawing_VBDecl_Color[] =
	{
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	const D3DVERTEXELEMENT9 g_Drawing_VDecl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	const int g_Drawing_nNumBatchInstance = 120;

	const int m_CheckRestoreEveryTicks = 5000;
	const int m_ThreadSleepMsIfNoData = 1;
	const uint32_t m_Version = 2;

	// Version: 3.0.3 (2017-10-31T10:37Z)
	// 
	class CDrawing_Functor
		: public CAfxFunctor
	{
	private:
		class CStatic
		{
		public:
			CStatic()
			{
				Update();
			}

			~CStatic()
			{
				if (m_VertexShader) m_VertexShader->Release();
				if (m_PixelShader) m_PixelShader->Release();
			}

			bool Active_get(void)
			{
				return m_Active;
			}

			void Console(IWrpCommandArgs * args)
			{
				m_Mutex.lock();

				if(DoConsole(args)) Update();

				m_Mutex.unlock();
			}

			void Draw(CDrawing_Functor const & functor)
			{
				m_Mutex.lock_shared();

				if (m_Active && m_Device && 0 < functor.m_Width && 0 < functor.m_Height)
				{
					if (m_Dirty)
					{
						CreateBuffers();
						m_Dirty = false;
					}

					if (m_PositionVertexBuffer && m_ColorVertexBuffer && m_IndexBuffer && m_VertexDecl)
					{
						if (!m_VertexShader) m_VertexShader = g_AfxShaders.GetAcsVertexShader(L"afx_pgldraw_vs20.acs", 0);
						if (!m_PixelShader) m_PixelShader = g_AfxShaders.GetAcsPixelShader(L"afx_pgldraw_ps20.acs", 0);

						IDirect3DVertexShader9 * vertexShader = m_VertexShader->GetVertexShader();
						IDirect3DPixelShader9 * pixelShader = m_PixelShader->GetPixelShader();

						if (vertexShader && pixelShader && UpdateCollorVertexBuffer(functor))
						{
							// Save device state:

							IDirect3DPixelShader9 * oldPixelShader = 0;
							m_Device->GetPixelShader(&oldPixelShader);
							if (oldPixelShader) oldPixelShader->AddRef();

							IDirect3DVertexShader9 * oldVertexShader = 0;
							m_Device->GetVertexShader(&oldVertexShader);
							if (oldVertexShader) oldVertexShader->AddRef();

							IDirect3DVertexBuffer9 * oldVertexBuffer0 = 0;
							UINT oldVertexBuffer0Offset;
							UINT oldVertexBuffer0Stride;
							m_Device->GetStreamSource(0, &oldVertexBuffer0, &oldVertexBuffer0Offset, &oldVertexBuffer0Stride);
							// this is done already according to doc: // if(oldVertexBuffer0) oldVertexBuffer0->AddRef();

							IDirect3DVertexBuffer9 * oldVertexBuffer1 = 0;
							UINT oldVertexBuffer1Offset;
							UINT oldVertexBuffer1Stride;
							m_Device->GetStreamSource(1, &oldVertexBuffer1, &oldVertexBuffer1Offset, &oldVertexBuffer1Stride);
							// this is done already according to doc: // if(oldVertexBuffer1) oldVertexBuffer1->AddRef();

							IDirect3DIndexBuffer9 * oldIndexBuffer = 0;
							m_Device->GetIndices(&oldIndexBuffer);
							// this is done already according to doc: // if(oldIndexBuffer) oldIndexBuffer->AddRef();

							IDirect3DVertexDeclaration9 * oldDeclaration;
							m_Device->GetVertexDeclaration(&oldDeclaration);
							if (oldDeclaration) oldDeclaration->AddRef();

							FLOAT oldCScreenInfo[4];
							m_Device->GetVertexShaderConstantF(48, oldCScreenInfo, 1);

							DWORD oldFVF;
							m_Device->GetFVF(&oldFVF);

							DWORD oldSrgbWriteEnable;
							m_Device->GetRenderState(D3DRS_SRGBWRITEENABLE, &oldSrgbWriteEnable);

							DWORD oldColorWriteEnable;
							m_Device->GetRenderState(D3DRS_COLORWRITEENABLE, &oldColorWriteEnable);

							DWORD oldZEnable;
							m_Device->GetRenderState(D3DRS_ZENABLE, &oldZEnable);

							DWORD oldAlphaTestEnable;
							m_Device->GetRenderState(D3DRS_ALPHATESTENABLE, &oldAlphaTestEnable);

							DWORD oldSeparateAlphaBlendEnable;
							m_Device->GetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, &oldSeparateAlphaBlendEnable);

							DWORD oldAlphaBlendEnable;
							m_Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlendEnable);

							DWORD oldCullMode;
							m_Device->GetRenderState(D3DRS_CULLMODE, &oldCullMode);

							DWORD oldMultiSampleAnitAlias;
							m_Device->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &oldMultiSampleAnitAlias);

							DWORD oldFillMode;
							m_Device->GetRenderState(D3DRS_FILLMODE, &oldFillMode);

							// Draw:
							{
								FLOAT newCScreenInfo[4] = { 2.0f / functor.m_Width, 2.0f / functor.m_Height, 0.0f, 0.0f };
								m_Device->SetVertexShaderConstantF(48, newCScreenInfo, 1);

								m_Device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
								m_Device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
								m_Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
								m_Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
								m_Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
								m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
								m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
								m_Device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
								m_Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

								m_Device->SetStreamSource(0, m_PositionVertexBuffer, 0, sizeof(VertexPosition));
								m_Device->SetStreamSource(1, m_ColorVertexBuffer, 0, sizeof(VertexColor));

								m_Device->SetVertexDeclaration(m_VertexDecl);
								m_Device->SetIndices(m_IndexBuffer);

								m_Device->SetVertexShader(vertexShader);
								m_Device->SetPixelShader(pixelShader);

								m_Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6 * m_RectsCount, 0, 2 * m_RectsCount);
							}

							// Restore device state:

							m_Device->SetPixelShader(oldPixelShader);
							if (oldPixelShader) oldPixelShader->Release();

							m_Device->SetVertexShader(oldVertexShader);
							if (oldVertexShader) oldVertexShader->Release();

							m_Device->SetStreamSource(1, oldVertexBuffer1, oldVertexBuffer1Offset, oldVertexBuffer1Stride);
							if (oldVertexBuffer1) oldVertexBuffer1->Release();

							m_Device->SetStreamSource(0, oldVertexBuffer0, oldVertexBuffer0Offset, oldVertexBuffer0Stride);
							if (oldVertexBuffer0) oldVertexBuffer0->Release();

							m_Device->SetIndices(oldIndexBuffer);
							if (oldIndexBuffer) oldIndexBuffer->Release();

							m_Device->SetFVF(oldFVF);

							m_Device->SetVertexDeclaration(oldDeclaration);
							if (oldDeclaration) oldDeclaration->Release();

							m_Device->SetRenderState(D3DRS_FILLMODE, oldFillMode);
							m_Device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, oldMultiSampleAnitAlias);
							m_Device->SetRenderState(D3DRS_CULLMODE, oldCullMode);
							m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlendEnable);
							m_Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, oldSeparateAlphaBlendEnable);
							m_Device->SetRenderState(D3DRS_ALPHATESTENABLE, oldAlphaTestEnable);
							m_Device->SetRenderState(D3DRS_ZENABLE, oldZEnable);
							m_Device->SetRenderState(D3DRS_COLORWRITEENABLE, oldColorWriteEnable);
							m_Device->SetRenderState(D3DRS_SRGBWRITEENABLE, oldSrgbWriteEnable);

							m_Device->SetVertexShaderConstantF(48, oldCScreenInfo, 1);
						}
					}
				}

				m_Mutex.unlock_shared();
			}

			void D3D9_BeginDevice(IDirect3DDevice9 * device)
			{
				D3D9_EndDevice();

				if (0 == device)
					return;

				{
					std::unique_lock<std::shared_timed_mutex> lock(m_Mutex);

					device->AddRef();

					m_Device = device;

					m_Dirty = true;
				}
			}

			void D3D9_EndDevice()
			{
				if (0 == m_Device)
					return;

				{
					std::unique_lock<std::shared_timed_mutex> lock(m_Mutex);

					DestroyBuffers();

					m_Device->Release();
					m_Device = 0;
				}
			}

			void D3D9_Reset()
			{
				{
					std::unique_lock<std::shared_timed_mutex> lock(m_Mutex);

					DestroyBuffers();
					m_Dirty = true;
				}
			}

		private:
			struct VertexPosition
			{
				FLOAT x, y; // Position of current line point
			};
			struct VertexColor
			{
				FLOAT r, g, b; // Diffuse color of current line point
			};

			bool m_Active = false;
			bool m_Dirty;
			IDirect3DDevice9 * m_Device = 0;
			IAfxVertexShader * m_VertexShader = 0;
			IAfxPixelShader * m_PixelShader = 0;

			std::shared_timed_mutex m_Mutex;

			class CChannel
			{
			public:
				bool Console(IWrpCommandArgs * args)
				{
					int argc = args->ArgC();

					char const * cmd0 = args->ArgV(0);

					if (4 <= argc)
					{
						m_Bits = max(1, (unsigned char)atoi(args->ArgV(1)));
						m_Lo = (float)atof(args->ArgV(2));
						m_Hi = (float)atof(args->ArgV(3));
						return true;
					}

					Tier0_Msg(
						"%s <iBits> <fLo> <fHi> - Set number of level bits, low value and high value for this channel.\n"
						"Current value: %u %f %f\n"
						, cmd0
						, m_Bits
						, m_Lo
						, m_Hi
					);

					return false;
				}

				unsigned short int Bits_get() const
				{
					return m_Bits;
				}

				float Encode(unsigned int value) const
				{
					return m_Lo + value / (float)((1u << m_Bits) - 1) * (m_Hi - m_Lo);
				}

			private:
				//float m_Lo = 0.0f / 255.0f;
				//float m_Hi = 255.0f / 255.0f;
				//unsigned short int m_Bits = 8;

				float m_Lo = 16.0f / 255.0f;
				float m_Hi = 235.0f / 255.0f;
				unsigned char m_Bits = 6;
			};

			CChannel m_Red;
			CChannel m_Green;
			CChannel m_Blue;

			bool DoConsole(IWrpCommandArgs * args)
			{
				int argc = args->ArgC();

				char const * cmd0 = args->ArgV(0);

				if (2 <= argc)
				{
					char const * cmd1 = args->ArgV(1);

					if (0 == _stricmp("start", cmd1))
					{
						m_Active = true;
						return true;
					}
					else if (0 == _stricmp("stop", cmd1))
					{
						m_Active = false;
						return true;
					}
					else if (0 == _stricmp("rectsPerRow", cmd1))
					{
						if (3 <= argc)
						{
							m_RectsPerRow = max(1, (size_t)atoi(args->ArgV(2)));
							return true;
						}

						Tier0_Msg(
							"%s rectsPerRow <i> - Number of rectangles per row.\n"
							"Current value: %u\n"
							, cmd0
							, m_RectsPerRow
						);
						return false;
					}
					else if (0 == _stricmp("rectWidth", cmd1))
					{
						if (3 <= argc)
						{
							m_RectWidth = (float)atof(args->ArgV(2));
							return true;
						}

						Tier0_Msg(
							"%s rectWidth <f> - Rectangle width.\n"
							"Current value: %f\n"
							, cmd0
							, m_RectWidth
						);
						return false;
					}
					else if (0 == _stricmp("rectHeight", cmd1))
					{
						if (3 <= argc)
						{
							m_RectHeight = (float)atof(args->ArgV(2));
							return true;
						}

						Tier0_Msg(
							"%s rectHeight <f> - Rectangle height.\n"
							"Current value: %f\n"
							, cmd0
							, m_RectHeight
						);
						return false;
					}
					else if (0 == _stricmp("top", cmd1))
					{
						if (3 <= argc)
						{
							m_Top = (float)atof(args->ArgV(2));
							return true;
						}

						Tier0_Msg(
							"%s top <f> - Top of output.\n"
							"Current value: %f\n"
							, cmd0
							, m_Top
						);
						return false;
					}
					else if (0 == _stricmp("left", cmd1))
					{
						if (3 <= argc)
						{
							m_Left = (float)atof(args->ArgV(2));
							return true;
						}

						Tier0_Msg(
							"%s left <f> - Left of output.\n"
							"Current value: %f\n"
							, cmd0
							, m_Left
						);
						return false;
					}
					else if (0 == _stricmp("red", cmd1))
					{
						CSubWrpCommandArgs subArgs(args, 2);

						return m_Red.Console(&subArgs);
					}
					else if (0 == _stricmp("green", cmd1))
					{
						CSubWrpCommandArgs subArgs(args, 2);

						return m_Green.Console(&subArgs);
					}
					else if (0 == _stricmp("blue", cmd1))
					{
						CSubWrpCommandArgs subArgs(args, 2);

						return m_Blue.Console(&subArgs);
					}
				}

				Tier0_Msg(
					"%s start - start drawing\n"
					"%s stop - stop drawing\n"
					"%s rectsPerRow [...]\n"
					"%s rectWidth [...]\n"
					"%s rectHeight [...]\n"
					"%s top [...]\n"
					"%s left [...]\n"
					"%s red [...]\n"
					"%s green [...]\n"
					"%s blue [...]\n"
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
					, cmd0
				);
				return false;
			}

			size_t m_RectsPerRow = 90;
			float m_RectWidth = 4.0f;
			float m_RectHeight = 4.0f;
			float m_Top = 75.0f;
			float m_Left = 10.0f;

			size_t m_RectsCount = 0;

			void Update()
			{
				m_Dirty = true;

				// If this gets larger, you need to make sure that the worst case bits doesn't blow up the indexbuffer!
				size_t bits = 0
					+1*32 // time
					+3*22 // position x y z
					+3*16 // rotation x y z
					+14 // fov
				; // == 160 bit == 20 byte

				size_t bitsPerRect = (size_t)m_Red.Bits_get() + (size_t)m_Green.Bits_get() + (size_t)m_Blue.Bits_get();
				
				m_RectsCount = bits / bitsPerRect + (0 != (bits % bitsPerRect) ? 1 : 0);
			}

			IDirect3DIndexBuffer9 * m_IndexBuffer = 0;
			IDirect3DVertexBuffer9 * m_PositionVertexBuffer = 0;
			IDirect3DVertexBuffer9 * m_ColorVertexBuffer = 0;
			IDirect3DVertexDeclaration9 * m_VertexDecl = 0;

			void Pack(float value, float min, float maxExclusive, unsigned char bits, size_t bitOfs, unsigned char * data)
			{
				value = (value - min) / (maxExclusive - min);

				unsigned int result = (unsigned int)min(max(std::floor(value * (1u << bits)),0), (1u << bits)-1);
				
				for (int bit = 0; bit < bits; ++bit)
				{
					size_t numByte = bitOfs / 8;
					size_t numBit = (7 - bitOfs) % 8;

					data[numByte] = (data[numByte] & ~(1u << numBit)) | (((result & (1u << (bits-1))) >> (bits -1)) << numBit);

					++bitOfs;
					result = result << 1;
				}
			}

			void Pack(float value, size_t bitOfs, unsigned char * data)
			{
				unsigned int result;

				Assert(sizeof(unsigned int) == sizeof(float));

				memcpy(&result, &value, sizeof(unsigned int));

				for (int bit = 0; bit < 32; ++bit)
				{
					size_t numByte = bitOfs / 8;
					size_t numBit = (7 - bitOfs) % 8;

					data[numByte] = (data[numByte] & ~(1u << numBit)) | (((result & (1u << (32 - 1))) >> (32 - 1)) << numBit);

					++bitOfs;
					result = result << 1;
				}
			}

			size_t Encode(CChannel const & channel, float & outValue, unsigned char * data, size_t maxBits, size_t bitOfs)
			{
				unsigned short int bits = channel.Bits_get();

				unsigned int value = 0;

				while (bits && bitOfs < maxBits)
				{
					size_t numByte = bitOfs / 8;
					size_t numBit = (7 - bitOfs) % 8;

					value = (value << 1) | ((data[numByte] & (1 << numBit)) >> numBit);

					++bitOfs;
					--bits;
				}

				outValue = channel.Encode(value);

				return bitOfs;
			}

			bool UpdateCollorVertexBuffer(CDrawing_Functor const & functor)
			{
				VertexColor * lockedColorBuffer;

				if(FAILED(m_ColorVertexBuffer->Lock(
					0,
					4 * m_RectsCount * sizeof(VertexColor),
					(void **)&lockedColorBuffer, 0
				)))
				{
					return false;
				}

				unsigned char data[20];

				Pack(functor.m_CamData.Time, 0, data);
				Pack(functor.m_CamData.XPosition, -16384, 16384 + FLT_EPSILON, 22, 32, data);
				Pack(functor.m_CamData.YPosition, -16384, 16384 + FLT_EPSILON, 22, 54, data);
				Pack(functor.m_CamData.ZPosition, -16384, 16384 + FLT_EPSILON, 22, 76, data);
				Pack((float)Afx::Math::AngleModDeg(functor.m_CamData.XRotation), -180, 180, 16, 98, data);
				Pack((float)Afx::Math::AngleModDeg(functor.m_CamData.YRotation), -180, 180, 16, 114, data);
				Pack((float)Afx::Math::AngleModDeg(functor.m_CamData.ZRotation), -180, 180, 16, 130, data);
				Pack(functor.m_CamData.Fov, 0, 180, 14, 146, data);

				size_t vertex = 0;
				size_t bitOfs = 0;
				for (size_t rect = 0; rect < m_RectsCount; ++rect)
				{
					float red, green, blue;

					bitOfs = Encode(m_Red, red, data, 160,
						Encode(m_Green, green, data, 160,
							Encode(m_Blue, blue, data, 160, bitOfs)
						)
					);

					lockedColorBuffer[vertex + 0].r = red;
					lockedColorBuffer[vertex + 0].g = green;
					lockedColorBuffer[vertex + 0].b = blue;

					lockedColorBuffer[vertex + 1].r = red;
					lockedColorBuffer[vertex + 1].g = green;
					lockedColorBuffer[vertex + 1].b = blue;

					lockedColorBuffer[vertex + 2].r = red;
					lockedColorBuffer[vertex + 2].g = green;
					lockedColorBuffer[vertex + 2].b = blue;

					lockedColorBuffer[vertex + 3].r = red;
					lockedColorBuffer[vertex + 3].g = green;
					lockedColorBuffer[vertex + 3].b = blue;

					vertex += 4;
				}

				m_ColorVertexBuffer->Unlock();

				return true;
			}

			void DestroyBuffers(void)
			{
				if (m_VertexDecl)
				{
					m_VertexDecl->Release();
					m_VertexDecl = 0;
				}

				if (m_PositionVertexBuffer)
				{
					m_PositionVertexBuffer->Release();
					m_PositionVertexBuffer = 0;
				}

				if (m_ColorVertexBuffer)
				{
					m_ColorVertexBuffer->Release();
					m_ColorVertexBuffer = 0;
				}

				if (m_IndexBuffer)
				{
					m_IndexBuffer->Release();
					m_IndexBuffer = 0;
				}
			}

			void CreateBuffers(void)
			{
				DestroyBuffers();

				if (FAILED(m_Device->CreateVertexDeclaration(g_Drawing_VDecl, &m_VertexDecl)))
				{
					if (m_VertexDecl)
					{
						m_VertexDecl->Release();
						m_VertexDecl = 0;
					}
				}

				size_t vertexCount = 4 * m_RectsCount;

				VertexPosition * lockedPositionBuffer;

				if (FAILED(m_Device->CreateVertexBuffer(
					vertexCount * sizeof(VertexPosition),
					D3DUSAGE_WRITEONLY,
					0,
					D3DPOOL_DEFAULT,
					&m_PositionVertexBuffer,
					NULL
				)) || FAILED(m_PositionVertexBuffer->Lock(
					0,
					vertexCount * sizeof(VertexPosition),
					(void **)&lockedPositionBuffer, 0
				)))
				{
					if (m_PositionVertexBuffer)
					{
						m_PositionVertexBuffer->Release();
						m_PositionVertexBuffer = 0;
					}
				}
				else
				{
					size_t row = 0;
					size_t col = 0;
					size_t vertex = 0;
					for (size_t rect = 0; rect < m_RectsCount; ++rect)
					{
						float top = -0.5f + m_Top + m_RectHeight * row;
						float left = -0.5f + m_Left + m_RectWidth * col;

						lockedPositionBuffer[vertex + 0].x = left;
						lockedPositionBuffer[vertex + 0].y = top;

						lockedPositionBuffer[vertex + 1].x = left + m_RectWidth;
						lockedPositionBuffer[vertex + 1].y = top;

						lockedPositionBuffer[vertex + 2].x = left;
						lockedPositionBuffer[vertex + 2].y = top + m_RectHeight;

						lockedPositionBuffer[vertex + 3].x = left + m_RectWidth;
						lockedPositionBuffer[vertex + 3].y = top + m_RectHeight;

						vertex += 4;

						++col;

						if (col % m_RectsPerRow == 0)
						{
							++row;
							col = 0;
						}
					}

					m_PositionVertexBuffer->Unlock();
				}

				if (FAILED(m_Device->CreateVertexBuffer(
					vertexCount * sizeof(VertexColor),
					D3DUSAGE_WRITEONLY,
					0,
					D3DPOOL_DEFAULT,
					&m_ColorVertexBuffer,
					NULL
				)))
				{
					if (m_ColorVertexBuffer)
					{
						m_ColorVertexBuffer->Release();
						m_ColorVertexBuffer = 0;
					}
				}


				size_t indexCount = 2 * 3 * m_RectsCount;

				if (65537 < indexCount)
					// Not possible on several graphic cards, sorry.
					return;

				WORD * lockedIndexBuffer;

				if (FAILED(m_Device->CreateIndexBuffer(
					indexCount * sizeof(WORD),
					D3DUSAGE_WRITEONLY,
					D3DFMT_INDEX16,
					D3DPOOL_DEFAULT,
					&m_IndexBuffer,
					NULL
				)) || FAILED(m_IndexBuffer->Lock(
					0,
					indexCount * sizeof(WORD),
					(void **)&lockedIndexBuffer,
					0
				)))
				{
					if (m_IndexBuffer)
					{
						m_IndexBuffer->Release();
						m_IndexBuffer = 0;
					}
				}
				else
				{
					WORD index = 0;
					for (WORD rect = 0; rect < m_RectsCount; ++rect)
					{
						lockedIndexBuffer[index + 0] = 4 * rect + 0;
						lockedIndexBuffer[index + 1] = 4 * rect + 1;
						lockedIndexBuffer[index + 2] = 4 * rect + 2;
						lockedIndexBuffer[index + 3] = 4 * rect + 1;
						lockedIndexBuffer[index + 4] = 4 * rect + 3;
						lockedIndexBuffer[index + 5] = 4 * rect + 2;

						index += 6;
					}

					m_IndexBuffer->Unlock();
				}
			}


		};
		
	private:
		static CStatic m_Static;

	public:
		static bool Active_get(void)
		{
			return m_Static.Active_get();
		}

		static void Console(IWrpCommandArgs * args)
		{
			m_Static.Console(args);
		}

		static void D3D9_BeginDevice(IDirect3DDevice9 * device)
		{
			m_Static.D3D9_BeginDevice(device);
		}

		static void D3D9_EndDevice()
		{
			m_Static.D3D9_EndDevice();
		}

		static void D3D9_Reset()
		{
			m_Static.D3D9_Reset();
		}

		CDrawing_Functor(CamData const & camData, int width, int height)
			: m_CamData(camData)
			, m_Width(width)
			, m_Height(height)
		{
		}

		virtual void operator()()
		{
			m_Static.Draw(*this);
		}

	private:
		CamData m_CamData;
		int m_Width;
		int m_Height;
	};

	MirvPgl::CDrawing_Functor::CStatic MirvPgl::CDrawing_Functor::m_Static;

	CamData::CamData()
	{

	}

	CamData::CamData(float time, float xPosition, float yPosition, float zPosition, float xRotation, float yRotation, float zRotation, float fov)
	: Time(time)
	, XPosition(xPosition)
	, YPosition(yPosition)
	, ZPosition(zPosition)
	, XRotation(xRotation)
	, YRotation(yRotation)
	, ZRotation(zRotation)
	, Fov(fov)
	{
	}

	class CThreadData
	{
	public:
		void Prepare(std::vector<std::uint8_t> const & data)
		{
			m_IsCancelled = false;
			m_Data = data;
		}

		void Cancel()
		{
			m_IsCancelled = true;
		}

		bool IsCancelled()
		{
			return m_IsCancelled;
		}

		std::vector<uint8_t> & AccessData()
		{
			return m_Data;
		}

	private:
		std::atomic_bool m_IsCancelled;
		std::vector<uint8_t> m_Data;
	};

	void DrawingThread_SupplyThreadData(CThreadData * threadData);

	class CSupplyThreadData_Functor
		: public CAfxFunctor
	{
	public:
		CSupplyThreadData_Functor(CThreadData * threadData)
			: m_Value(threadData)
		{
		}

		virtual void operator()()
		{
			DrawingThread_SupplyThreadData(m_Value);
		}

	private:
		CThreadData * m_Value;
	};


	class CThreadDataPool
	{
	public:
		/// <remarks>Must only be called from main thread.</remarks>
		std::vector<std::uint8_t> & AccessNextThreadData(void)
		{
			return m_Data;
		}

		/// <remarks>Must only be called from main thread.</remarks>
		CThreadData * Acquire(void)
		{
			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			CThreadData * threadData;
			
			if (m_ThreadDataAvailable.empty())
			{
				threadData = new CThreadData();
			}
			else
			{
				threadData = m_ThreadDataAvailable.front();
				m_ThreadDataAvailable.pop();
			}

			threadData->Prepare(m_Data);

			m_Data.clear();

			m_ThreadDataInUse.insert(threadData);

			return threadData;
		}

		void Return(CThreadData * threadData)
		{
			threadData->Cancel();

			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			m_ThreadDataInUse.erase(threadData);

			m_ThreadDataAvailable.push(threadData);
		}

		void Cancel(void)
		{
			std::unique_lock<std::mutex> lock(m_ThreadDataQueueMutex);

			for (std::set<CThreadData *>::iterator it = m_ThreadDataInUse.begin(); it != m_ThreadDataInUse.end(); ++it)
			{
				(*it)->Cancel();
			}

			m_Data.clear();
		}

	private:
		std::vector<std::uint8_t> m_Data;
		std::queue<CThreadData *> m_ThreadDataAvailable;
		std::set<CThreadData *> m_ThreadDataInUse;
		std::mutex m_ThreadDataQueueMutex;
	} m_ThreadDataPool;

	bool m_WsaActive = false;

	WebSocket * m_Ws = 0;
	bool m_WantWs = false;
	std::string m_WsUrl("ws://host:port/path");
	std::mutex m_WsMutex;

	std::list<std::string> m_Commands;
	std::mutex m_CommandsMutex;

	std::mutex m_CamOverrideMutex;
	bool m_CamOverride = false;
	float m_CamXPosition = 0;
	float m_CamYPosition = 0;
	float m_CamZPosition = 0;
	float m_CamXRotation = 0;
	float m_CamYRotation = 0;
	float m_CamZRotation = 0;
	float m_CamFov = 90;

	bool OnViewOverride(float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov) {
		std::unique_lock<std::mutex> lock(m_CamOverrideMutex);
		if (m_CamOverride) {
			Tx = m_CamXPosition;
			Ty = m_CamYPosition;
			Tz = m_CamZPosition;
			Rz = m_CamXRotation;
			Rx = m_CamYRotation;
			Ry = m_CamZRotation;
			Fov = Auto_InverseFovScaling(g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, m_CamFov);
			return true;
		}
		return false;
	}

	std::vector<uint8_t> m_SendThreadTempData;

	std::vector<uint8_t> m_DataForSendThread;
	std::condition_variable m_DataForSendThreadAvailableCondition;
	std::mutex m_DataForSendThreadMutex;
	bool m_InTransaction;

	bool m_DataActive = false;

	std::thread * m_Thread = 0;
	bool m_WantClose = false;

	DWORD m_LastCheckRestoreTick = 0;

	CThreadData * m_DrawingThread_ThreadData = 0;

	std::string m_CurrentLevel;

	void AppendCString(char const * cstr, std::vector<uint8_t> &outVec)
	{
		std::string str(cstr);
		outVec.insert(outVec.end(), str.begin(), str.end());
		outVec.push_back(static_cast<uint8_t>('\0'));
	}

	void AppendFloat(float_t value, std::vector<uint8_t> &outVec)
	{
		uint8_t data[sizeof(value)];
		memcpy(&(data[0]), &value, sizeof(value));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendInt32(int32_t value, std::vector<uint8_t> &outVec)
	{
		uint8_t data[sizeof(value)];
		memcpy(&(data[0]), &value, sizeof(value));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendInt16(int16_t value, std::vector<uint8_t> &outVec)
	{
		uint8_t data[sizeof(value)];
		memcpy(&(data[0]), &value, sizeof(value));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendByte(uint8_t value, std::vector<uint8_t> &outVec)
	{
		outVec.push_back(value);
	}

	void AppendBoolean(uint8_t value, std::vector<uint8_t> &outVec)
	{
		AppendByte(value ? 1 : 0, outVec);
	}

	void AppendUInt64(uint64_t value, std::vector<uint8_t> &outVec)
	{
		uint8_t data[sizeof(value)];
		memcpy(&(data[0]), &value, sizeof(value));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendHello(std::vector<uint8_t> &outVec)
	{
		uint8_t data[1 * sizeof(uint32_t)];
		uint32_t version = m_Version;

		AppendCString("hello", outVec);
		memcpy(&(data[0 * sizeof(uint32_t)]), &version, sizeof(uint32_t));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void AppendCamData(CamData const camData, std::vector<uint8_t> &outVec)
	{
		uint8_t data[8 * sizeof(float)];

		memcpy(&(data[0 * sizeof(float)]), &camData.Time, sizeof(float));
		memcpy(&(data[1 * sizeof(float)]), &camData.XPosition, sizeof(float));
		memcpy(&(data[2 * sizeof(float)]), &camData.YPosition, sizeof(float));
		memcpy(&(data[3 * sizeof(float)]), &camData.ZPosition, sizeof(float));
		memcpy(&(data[4 * sizeof(float)]), &camData.XRotation, sizeof(float));
		memcpy(&(data[5 * sizeof(float)]), &camData.YRotation, sizeof(float));
		memcpy(&(data[6 * sizeof(float)]), &camData.ZRotation, sizeof(float));
		memcpy(&(data[7 * sizeof(float)]), &camData.Fov, sizeof(float));

		outVec.insert(outVec.end(), std::begin(data), std::end(data));
	}

	void Recv_String(const std::string & message)
	{
		// lul
	}

	void Recv_Bytes(const std::vector<uint8_t>& message)
	{
		std::vector<uint8_t>::const_iterator itBegin = message.begin();

		while (itBegin != message.end())
		{
			std::vector<uint8_t>::const_iterator itDelim = message.end();

			for (std::vector<uint8_t>::const_iterator it = itBegin; it != message.end(); ++it)
			{
				if ((uint8_t)'\0' == *it)
				{
					itDelim = it;
					break;
				}
			}

			if (message.end() != itDelim && itBegin != itDelim)
			{
				std::string strCode(itBegin, itDelim);

				char const * code = strCode.c_str();

				if (0 == strcmp("setCam", code) && message.size() >= strCode.size() + 1 + 7*sizeof(float)) {
					size_t ofs = strCode.size() + 1;
					std::unique_lock<std::mutex> lock(m_CamOverrideMutex);
					m_CamOverride = true;
					memcpy(&m_CamXPosition, (float*)&message[ofs + 0], sizeof(float));
					memcpy(&m_CamYPosition, (float*)&message[ofs + 1 * sizeof(float)], sizeof(float));
					memcpy(&m_CamZPosition, (float*)&message[ofs + 2 * sizeof(float)], sizeof(float));
					memcpy(&m_CamXRotation, (float*)&message[ofs + 3 * sizeof(float)], sizeof(float));
					memcpy(&m_CamYRotation, (float*)&message[ofs + 4 * sizeof(float)], sizeof(float));
					memcpy(&m_CamZRotation, (float*)&message[ofs + 5 * sizeof(float)], sizeof(float));
					memcpy(&m_CamFov, (float*)&message[ofs + 6 * sizeof(float)], sizeof(float));
				}
				if (0 == strcmp("setCamEnd", code)) {
					std::unique_lock<std::mutex> lock(m_CamOverrideMutex);
					m_CamOverride = false;
				}
				else if (0 == strcmp("exec", code))
				{
					std::unique_lock<std::mutex> lock(m_CommandsMutex);

					std::vector<uint8_t>::const_iterator itCmdStart = itDelim + 1;
					std::vector<uint8_t>::const_iterator itCmdEnd = itCmdStart;

					bool foundDelim = false;

					for (std::vector<uint8_t>::const_iterator it = itCmdStart; it != message.end(); ++it)
					{
						if ((uint8_t)'\0' == *it)
						{
							foundDelim = true;
							itCmdEnd = it;
							break;
						}					
					}

					if (!foundDelim)
						break;

					std::string cmds(itCmdStart, itCmdEnd);

					m_Commands.push_back(cmds);

					itBegin = itCmdEnd + 1;

					continue;
				}
				else if (0 == strcmp("transBegin", code))
				{
					m_InTransaction = true;
					itBegin = itDelim + 1;
				}
				else if (0 == strcmp("transEnd", code))
				{
					itBegin = itDelim + 1;
					m_InTransaction = false;
				}
			}

			break;
		}
	}

	void Thread()
	{
		m_SendThreadTempData.clear();

		while (true)
		{
			{
				std::unique_lock<std::mutex> wsLock(m_WsMutex);

				if (WebSocket::CLOSED == m_Ws->getReadyState())
				{
					delete m_Ws;
					m_Ws = 0;
					break;
				}
			}

			m_Ws->poll();

			// this would eat our shit: m_Ws->dispatch(Recv_String); 
			m_Ws->dispatchBinary(Recv_Bytes);

			std::unique_lock<std::mutex> dataLock(m_DataForSendThreadMutex);

			m_DataForSendThreadAvailableCondition.wait_for(dataLock, m_ThreadSleepMsIfNoData * 1ms, [] { return !m_DataForSendThread.empty() || m_WantClose; }); // if we don't need to send data, we are a bit lazy in order to save some CPU. Of course this assumes, that the data we get from network can wait that long ;)

			if (!m_DataForSendThread.empty())
			{
				m_SendThreadTempData = std::move(m_DataForSendThread);
				m_DataForSendThread.clear();
				dataLock.unlock();

				m_Ws->sendBinary(m_SendThreadTempData);

				m_SendThreadTempData.clear();
			}
			else
				dataLock.unlock();

			if (m_WantClose)
				m_Ws->close();
		}
	}

	void EndThread()
	{
		if (0 != m_Thread)
		{
			m_WantClose = true;
			
			m_DataForSendThreadAvailableCondition.notify_one();

			m_Thread->join();
			
			delete m_Thread;
			m_Thread = 0;
			
			m_WantClose = false;
		}
	}

	void CreateThread()
	{
		EndThread();

		m_InTransaction = false;
		m_Thread = new std::thread(Thread);
	}


	void Init()
	{
		Shutdown();

		WSADATA wsaData;

		m_WsaActive = 0 == WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	void Shutdown()
	{
		Stop();

		if (m_WsaActive)
		{
			WSACleanup();
			m_WsaActive = false;
		}
	}

	void Url_set(char const * url)
	{
		m_WsUrl = url;
	}

	char const * Url_get(void)
	{
		return m_WsUrl.c_str();
	}

	void Restart_MirvPglGameEventSerializer();

	void Start()
	{
		Stop();

		if(m_WsaActive)
		{
			m_WantWs = true;

			m_Ws = WebSocket::from_url(m_WsUrl);

			if (0 != m_Ws)
			{
				AppendHello(m_ThreadDataPool.AccessNextThreadData());

				Restart_MirvPglGameEventSerializer();

				CreateThread();
			}
		}
	}

	void Stop()
	{
		m_DataActive = false;
		m_ThreadDataPool.Cancel();
		m_ThreadDataPool.AccessNextThreadData().clear();

		EndThread();

		m_WantWs = false;
	}

	bool IsStarted()
	{
		return m_WantWs;
	}

	void DataStart()
	{
		DataStop();

		if (m_WantWs)
		{
			m_DataActive = true;

			std::vector<uint8_t> & data = m_ThreadDataPool.AccessNextThreadData();

			AppendCString("dataStart", data);

			if (!m_CurrentLevel.empty())
			{

				AppendCString("levelInit", data);
				AppendCString(m_CurrentLevel.c_str(), data);
			}
		}
	}

	void DataStop()
	{
		if (m_DataActive)
		{
			m_DataActive = false;

			m_ThreadDataPool.Cancel();

			if (m_WantWs)
			{
				AppendCString("dataStop", m_ThreadDataPool.AccessNextThreadData());
			}
		}
	}

	bool IsDataActive()
	{
		return m_DataActive;
	}

	bool IsDrawingActive()
	{
		return CDrawing_Functor::Active_get();
	}

	void CheckStartedAndRestoreIfDown()
	{
		if (m_WantWs)
		{
			DWORD curTick = GetTickCount();

			if (m_CheckRestoreEveryTicks <= abs((int)m_LastCheckRestoreTick - (int)curTick))
			{
				m_LastCheckRestoreTick = curTick;

				bool needRestore = false;
				{
					std::unique_lock<std::mutex> lock(m_WsMutex);

					needRestore = 0 == m_Ws;
				}

				if (needRestore)
				{
					Start();
				}
			}
		}
	}

	void QueueThreadDataForDrawingThread(void)
	{
		if(m_DataActive || !m_ThreadDataPool.AccessNextThreadData().empty())
			QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CSupplyThreadData_Functor(m_ThreadDataPool.Acquire())));
	}

	void QueueDrawing(CamData const & camData, int width, int height)
	{
		QueueOrExecute(GetCurrentContext()->GetOrg(), new CAfxLeafExecute_Functor(new CDrawing_Functor(camData, width, height)));
	}

	void SupplyLevelInit(char const * mapName)
	{
		m_CurrentLevel = mapName;

		if (!m_DataActive)
			return;

		std::vector<uint8_t> & data = m_ThreadDataPool.AccessNextThreadData();

		AppendCString("levelInit", data);
		AppendCString(mapName, data);
	}

	void SupplyLevelShutdown()
	{
		m_CurrentLevel.clear();

		if (!m_DataActive)
			return;

		AppendCString("levelShutdown", m_ThreadDataPool.AccessNextThreadData());
	}

	void ExecuteQueuedCommands()
	{
		std::unique_lock<std::mutex> lock(m_CommandsMutex);

		while (0 < m_Commands.size())
		{
			std::string cmd = m_Commands.front();
			m_Commands.pop_front();
			lock.unlock();

			g_VEngineClient->ExecuteClientCmd(cmd.c_str());

			lock.lock();
		}
	}

	void DrawingThread_SupplyThreadData(CThreadData * threadData)
	{
		if (m_DrawingThread_ThreadData)
		{
			// should not happen.
			Assert(0);

			m_ThreadDataPool.Return(m_DrawingThread_ThreadData);
		}

		m_DrawingThread_ThreadData = threadData;
	}

	void D3D9_BeginDevice(IDirect3DDevice9 * device)
	{
		CDrawing_Functor::D3D9_BeginDevice(device);
	}

	void D3D9_EndDevice()
	{
		CDrawing_Functor::D3D9_EndDevice();
	}
	
	void D3D9_Reset()
	{
		CDrawing_Functor::D3D9_Reset();
	}

	void DrawingThread_SupplyCamData(CamData const & camData)
	{
		if(m_DrawingThread_ThreadData)
		{
			std::vector<uint8_t> & data = m_DrawingThread_ThreadData->AccessData();

			AppendCString("cam", data);
			AppendCamData(camData, data);
		}
	}

	void DrawingThread_UnleashData()
	{
		if (m_DrawingThread_ThreadData && !m_DrawingThread_ThreadData->IsCancelled())
		{
			std::vector<uint8_t> & data = m_DrawingThread_ThreadData->AccessData();

			std::unique_lock<std::mutex> lock(m_DataForSendThreadMutex);

			m_DataForSendThread.insert(m_DataForSendThread.end(), data.begin(), data.end());

			lock.unlock();

			m_DataForSendThreadAvailableCondition.notify_one();

			m_ThreadDataPool.Return(m_DrawingThread_ThreadData);

			m_DrawingThread_ThreadData = 0;
		}
	}

	class CMirvPglGameEventSerializer : public CAfxGameEventListenerSerialzer
	{
	public:
		virtual bool BeginSerialize() override
		{
			if (!m_WantWs)
				return false;

			m_Data = &(m_ThreadDataPool.AccessNextThreadData());

			AppendCString("gameEvent", *m_Data);

			return true;
		}

		virtual void EndSerialize() override
		{

		}

		virtual void WriteCString(const char * value) override
		{
			AppendCString(value, *m_Data);
		}

		virtual void WriteFloat(float value) override
		{
			AppendFloat(value, *m_Data);
		}

		virtual void WriteLong(long value) override
		{
			AppendInt32(value, *m_Data);
		}

		virtual void WriteShort(short value) override
		{
			AppendInt16(value, *m_Data);
		}

		virtual void WriteByte(char value)
		{
			AppendByte((uint8_t)value, *m_Data);
		}

		virtual void WriteBoolean(bool value)
		{
			AppendBoolean(value, *m_Data);
		}

		virtual void WriteUInt64(unsigned __int64 value)
		{
			AppendUInt64(value, *m_Data);
		}

	private:
		std::vector<uint8_t> * m_Data;
	} g_MirvPglGameEventSerializer;

	void Restart_MirvPglGameEventSerializer()
	{
		g_AfxGameEvents.RemoveListener(&MirvPgl::g_MirvPglGameEventSerializer);
		g_MirvPglGameEventSerializer.Restart();
		g_MirvPglGameEventSerializer.SetUseGameEventCache(false);
	}
}

CON_COMMAND(mirv_pgl, "PGL")
{
	int argc = args->ArgC();

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("start", cmd1))
		{
			if (!MirvPgl::m_WsaActive)
			{
				Tier0_Warning("Error: WinSock(2.2) not active, feature not available!\n");
				return;
			}

			MirvPgl::Start();
			return;
		}
		else if (0 == _stricmp("stop", cmd1))
		{
			MirvPgl::Stop();
			return;
		}
		else if (0 == _stricmp("dataStart", cmd1))
		{
			MirvPgl::DataStart();
			return;
		}
		else if (0 == _stricmp("dataStop", cmd1))
		{
			MirvPgl::DataStop();
			return;
		}
		else if (0 == _stricmp("url", cmd1))
		{
			if (3 <= argc)
			{
				MirvPgl::Url_set(args->ArgV(2));
				return;
			}

			Tier0_Msg(
				"mirv_pgl url <url> - Set url to use with start.\n"
				"Current value: %s\n"
				, MirvPgl::Url_get()
			);
			return;
		}
		else if (0 == _stricmp("draw", cmd1))
		{
			CSubWrpCommandArgs subArgs(args, 2);

			MirvPgl::CDrawing_Functor::Console(&subArgs);

			return;
		}
		else if (0 == _stricmp(cmd1, "events"))
		{
			if (3 <= argc)
			{
				const char * arg2 = args->ArgV(2);

				if (0 == _stricmp(arg2, "enabled") && 4 <= argc)
				{
					bool enable = 0 != atoi(args->ArgV(3));

					if (enable)
						g_AfxGameEvents.AddListener(&MirvPgl::g_MirvPglGameEventSerializer);
					else
						g_AfxGameEvents.RemoveListener(&MirvPgl::g_MirvPglGameEventSerializer);

					return;
				}
				else if (0 == _stricmp(arg2, "useCache") && 4 <= argc)
				{
					bool enable = 0 != atoi(args->ArgV(3));

					MirvPgl::g_MirvPglGameEventSerializer.SetUseGameEventCache(enable);

					return;
				}
				else if (0 == _stricmp(arg2, "whitelist") && 4 <= argc)
				{
					const char * arg3 = args->ArgV(3);

					if (0 == _stricmp(arg3, "clear"))
					{
						MirvPgl::g_MirvPglGameEventSerializer.ClearWhiteList();
						return;
					}
					else if (0 == _stricmp(arg3, "add") && 5 <= argc)
					{
						const char * arg4 = args->ArgV(4);

						MirvPgl::g_MirvPglGameEventSerializer.WhiteList(arg4);
						return;
					}
					else if (0 == _stricmp(arg3, "remove") && 5 <= argc)
					{
						const char * arg4 = args->ArgV(4);

						MirvPgl::g_MirvPglGameEventSerializer.UnWhiteList(arg4);
						return;
					}
				}
				else if (0 == _stricmp(arg2, "blacklist") && 4 <= argc)
				{
					const char * arg3 = args->ArgV(3);

					if (0 == _stricmp(arg3, "clear"))
					{
						MirvPgl::g_MirvPglGameEventSerializer.ClearBlackList();
						return;
					}
					else if (0 == _stricmp(arg3, "add") && 5 <= argc)
					{
						const char * arg4 = args->ArgV(4);

						MirvPgl::g_MirvPglGameEventSerializer.BlackList(arg4);
						return;
					}
					else if (0 == _stricmp(arg3, "remove") && 5 <= argc)
					{
						const char * arg4 = args->ArgV(4);

						MirvPgl::g_MirvPglGameEventSerializer.BlackList(arg4);
						return;
					}
				}
				else if (0 == _stricmp(arg2, "enrich") && 4 <= argc)
				{
					const char * arg3 = args->ArgV(3);

					if (0 == _stricmp(arg3, "clear"))
					{
						MirvPgl::g_MirvPglGameEventSerializer.ClearEnrichments();
						return;
					}
					else if (0 == _stricmp(arg3, "eventProperty") && 7 <= argc)
					{
						const char * arg4 = args->ArgV(4);
						const char * arg5 = args->ArgV(5);
						const char * arg6 = args->ArgV(6);

						if (0 == _stricmp(arg4, "useridWithSteamId"))
						{
							MirvPgl::g_MirvPglGameEventSerializer.EnrichUseridWithSteamId(arg5, arg6);
							return;
						}
						else if (0 == _stricmp(arg4, "entnumWithOrigin"))
						{
							MirvPgl::g_MirvPglGameEventSerializer.Enrich_EntnumWithOrigin(arg5, arg6);
							return;
						}
						else if (0 == _stricmp(arg4, "entnumWithAngles"))
						{
							MirvPgl::g_MirvPglGameEventSerializer.Enrich_EntnumWithAngles(arg5, arg6);
							return;
						}
						else if (0 == _stricmp(arg4, "useridWithEyePosition"))
						{
							MirvPgl::g_MirvPglGameEventSerializer.Enrich_UseridWithEyePosition(arg5, arg6);
							return;
						}
						else if (0 == _stricmp(arg4, "useridWithEyeAngles"))
						{
							MirvPgl::g_MirvPglGameEventSerializer.Enrich_UseridWithEyeAngels(arg5, arg6);
							return;
						}
					}
					else if (0 == _stricmp(arg3, "clientTime") && 5 <= argc)
					{
						MirvPgl::g_MirvPglGameEventSerializer.TransmitClientTime = 0 != atoi(args->ArgV(4));
						return;
					}
					else if (0 == _stricmp(arg3, "tick") && 5 <= argc)
					{
						MirvPgl::g_MirvPglGameEventSerializer.TransmitTick = 0 != atoi(args->ArgV(4));
						return;
					}
					else if (0 == _stricmp(arg3, "systemtime") && 5 <= argc)
					{
						MirvPgl::g_MirvPglGameEventSerializer.TransmitClientTime = 0 != atoi(args->ArgV(4));
						return;
					}
				}
			}

			Tier0_Msg(
				"mirv_pgl events enabled 0|1\n"
				"mirv_pgl events useCache 0|1\n"
				"mirv_pgl events whitelist clear\n"
				"mirv_pgl events whitelist add <eventName>\n"
				"mirv_pgl events whitelist remove <eventName>\n"
				"mirv_pgl events blacklist clear\n"
				"mirv_pgl events blacklist add <eventName>\n"
				"mirv_pgl events blacklist remove <eventName>\n"
				"mirv_pgl events enrich clear\n"
				"mirv_pgl events enrich eventProperty useridWithSteamId|entnumWithOrigin|entnumWithAngles|useridWithEyePosition|useridWithEyeAngles <eventName> <property>\n"
				"mirv_pgl events enrich clientTime 0|1\n"
				"mirv_pgl events enrich tick 0|1\n"
				"mirv_pgl events enrich systemtime 0|1\n"
			);
			return;
		}
	}

	Tier0_Msg(
		"mirv_pgl start - (Re-)Starts connection to websocket server.\n"
		"mirv_pgl stop - Stops connection to websocket server.\n"
		"mirv_pgl dataStart - Start sending data.\n"
		"mirv_pgl dataStop - Stop sending data.\n"
		"mirv_pgl url [...] - Set url to use with start.\n"
		"mirv_pgl draw [...] - Controls on-screen data drawing.\n"
		"mirv_pgl events [...] - Control game event data (disabled by default, requires start with version 3 or newer).\n"
	);

}


#endif // ifdef AFX_MIRV_PGL

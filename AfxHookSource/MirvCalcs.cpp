#include "stdafx.h"

#include "MirvCalcs.h"

#include "WrpVEngineClient.h"
#include "WrpConsole.h"
#include "RenderView.h"
#include "addresses.h"
#include "CamIO.h"

#include <shared/StringTools.h>
#include <ctype.h>

extern WrpVEngineClient * g_VEngineClient;

CMirvHandleCalcs g_MirvHandleCalcs;
CMirvVecAngCalcs g_MirvVecAngCalcs;
CMirvCamCalcs g_MirvCamCalcs;
CMirvFovCalcs g_MirvFovCalcs;

class CMirvCalc
{
public:
	CMirvCalc(char const * name)
		: m_Name(name ? name : "(no name)")
	{

	}

	virtual void AddRef(void)
	{
		++m_RefCount;
	}

	virtual void Release(void)
	{
		int cnt = --m_RefCount;

		if (0 == cnt)
			delete this;
	}

	virtual int GetRefCount(void)
	{
		return m_RefCount;
	}

	virtual  char const * GetName(void)
	{
		return m_Name.c_str();
	}

	virtual void Console_Print(void)
	{
		Tier0_Msg("name=\"%s\"", m_Name.c_str());
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		Tier0_Msg("No editable options.\n");
	}

protected:
	virtual ~CMirvCalc()
	{

	}

private:
	std::string m_Name;
	int m_RefCount = 0;
};

class CMirvHandleCalc : public CMirvCalc, public IMirvHandleCalc
{
public:
	CMirvHandleCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_Print(void)
	{
		CMirvCalc::Console_Print();
	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvHandleValueCalc : public CMirvHandleCalc
{
public:
	CMirvHandleValueCalc(char const * name, int handle)
		: CMirvHandleCalc(name)
		, m_Handle(handle)
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		outHandle = m_Handle;
		return true;
	}

	virtual void Console_Print(void)
	{
		CMirvHandleCalc::Console_Print();

		Tier0_Msg(" type=value handle=%i", m_Handle);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);
		
		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("handle", arg1))
			{
				if (3 <= argc)
				{
					m_Handle = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s handle <iHandle> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Handle.ToInt()
				);
				return;
			}
		}

		Tier0_Msg(
			"%s handle [...]\n"
			, arg0
		);
	}

private:
	SOURCESDK::CSGO::CBaseHandle m_Handle;
};

class CMirvHandleIndexCalc : public CMirvHandleCalc
{
public:
	CMirvHandleIndexCalc(char const * name, int index)
		: CMirvHandleCalc(name)
		, m_Index(index)
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(m_Index);

		if (ce)
		{
			outHandle = ce->GetRefEHandle();
			return true;
		}

		return false;
	}

	virtual void Console_Print(void)
	{
		CMirvHandleCalc::Console_Print();

		Tier0_Msg(" type=index index=%i", m_Index);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("index", arg1))
			{
				if (3 <= argc)
				{
					m_Index = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s index <iIndex> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Index
				);
				return;
			}
		}

		Tier0_Msg(
			"%s index [...]\n"
			, arg0
		);
	}

private:
	int m_Index;
};

typedef bool(*csgon_fnPlayerSidesWappedOnScreen_t)(void);

class CMirvHandleKeyCalc : public CMirvHandleCalc
{
public:
	CMirvHandleKeyCalc(char const * name, int key)
		: CMirvHandleCalc(name)
		, m_Key(key)
		, m_ClSpecSwapPlayerSides("cl_spec_swapplayersides")
	{

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		// Left screen side keys: 1, 2, 3, 4, 5
		// Right screen side keys: 6, 7, 8, 9, 0

		csgon_fnPlayerSidesWappedOnScreen_t fnPlayerSidesWappedOnScreen = (csgon_fnPlayerSidesWappedOnScreen_t)AFXADDR_GET(csgo_Unknown_GetTeamsSwappedOnScreen);

		bool swapPlayerSide = fnPlayerSidesWappedOnScreen && fnPlayerSidesWappedOnScreen();

		int nr = ((m_Key +9) % 10);

		bool isOtherScreenSide = 0 != ((nr / 5) % 2);

		int slot = (nr % 5);

		int slotCT = 0;
		int slotT = 0;

		WrpGlobals * gl = g_Hook_VClient_RenderView.GetGlobals();
		int imax = gl ? gl->maxclients_get() : 0;

		for (int i = 1; i <= imax; ++i)
		{
			SOURCESDK::IClientEntity_csgo * ce = SOURCESDK::g_Entitylist_csgo->GetClientEntity(i);
			SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;
			if (be && be->IsPlayer())
			{
				int be_team = be->GetTeamNumber();

				if (3 == be_team) // CT
				{
					if (isOtherScreenSide == swapPlayerSide && slotCT == slot)
					{
						outHandle = ce->GetRefEHandle();
						return true;
					}

					++slotCT;
				}
				else if (2 == be_team) // T
				{
					if (isOtherScreenSide != swapPlayerSide && slotT == slot)
					{
						outHandle = ce->GetRefEHandle();
						return true;
					}

					++slotT;
				}
			}
		}

		return false;
	}

	virtual void Console_Print(void)
	{
		CMirvHandleCalc::Console_Print();

		Tier0_Msg(" type=key key=%i", m_Key);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("key", arg1))
			{
				if (3 <= argc)
				{
					m_Key = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s key <iKey> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Key
				);
				return;
			}
		}

		Tier0_Msg(
			"%s key [...]\n"
			, arg0
		);
	}

private:
	int m_Key;
	WrpConVarRef m_ClSpecSwapPlayerSides;
};

class CMirvHandleActiveWeaponCalc : public CMirvHandleCalc
{
public:
	CMirvHandleActiveWeaponCalc(char const * name, IMirvHandleCalc * parentCalc, bool world)
		: CMirvHandleCalc(name)
		, m_ParentCalc(parentCalc)
		, m_World(world)
	{
		m_ParentCalc->AddRef();

	}

	virtual bool CalcHandle(SOURCESDK::CSGO::CBaseHandle & outHandle)
	{
		SOURCESDK::CSGO::CBaseHandle parentHandle;

		if (m_ParentCalc->CalcHandle(parentHandle) && parentHandle.IsValid())
		{
			SOURCESDK::IClientEntity_csgo * clientEntity = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(parentHandle);
			SOURCESDK::C_BaseEntity_csgo * baseEntity = clientEntity ? clientEntity->GetBaseEntity() : 0;
			SOURCESDK::C_BaseCombatCharacter_csgo * combatCharacter = baseEntity ? baseEntity->MyCombatCharacterPointer() : 0;
			SOURCESDK::C_BaseCombatWeapon_csgo * activeWeapon = combatCharacter ? combatCharacter->GetActiveWeapon() : 0;

			if (activeWeapon)
			{
				SOURCESDK::IClientEntity_csgo * ce = 0;
				
				if(!m_World)
					ce = activeWeapon->GetIClientEntity();
				else if(-1 != AFXADDR_GET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel))
				{
					SOURCESDK::CSGO::CBaseHandle hWeaponWorldModel = *(SOURCESDK::CSGO::CBaseHandle *)((char const *)activeWeapon + AFXADDR_GET(csgo_C_BaseCombatWeapon_m_hWeaponWorldModel));
					ce = SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(hWeaponWorldModel);
				}

				if (ce)
				{
					outHandle = ce->GetRefEHandle();
					return true;
				}
			}
		}

		return false;
	}

	virtual void Console_Print(void)
	{
		CMirvHandleCalc::Console_Print();

		Tier0_Msg(" type=activeWeapon parent=\"%s\", getWorld=%i", m_ParentCalc->GetName(), m_World ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("getWorld", arg1))
			{
				if (3 <= argc)
				{
					m_World = 0 != atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s getWorld <bGetWorld> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_World ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s getWorld [...]\n"
			, arg0
		);
	}

protected:
	virtual ~CMirvHandleActiveWeaponCalc()
	{
		m_ParentCalc->Release();
	}

private:
	IMirvHandleCalc * m_ParentCalc;
	bool m_World;
};



class CMirvVecAngCalc : public CMirvCalc, public IMirvVecAngCalc
{
public:
	CMirvVecAngCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_Print(void)
	{
		CMirvCalc::Console_Print();
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};


class CMirvCamCalc : public CMirvCalc, public IMirvCamCalc
{
public:
	CMirvCamCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_Print(void)
	{
		CMirvCalc::Console_Print();
	}

	virtual bool CalcFov(float & outFov)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

class CMirvFovCalc : public CMirvCalc, public IMirvFovCalc
{
public:
	CMirvFovCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_Print(void)
	{
		CMirvCalc::Console_Print();
	}

	virtual bool CalcFov(float & outFov)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

class CMirvVecAngValueCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngValueCalc(char const * name, float x, float y, float z, float rX, float rY, float rZ)
		: CMirvVecAngCalc(name)
	{
		m_Vec.x = x;
		m_Vec.y = y;
		m_Vec.z = z;
		m_Ang.z = rX;
		m_Ang.x = rY;
		m_Ang.y = rZ;
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=value x=%f y=%f, z=%f, rX=%f, rY=%f, rZ=%f", m_Vec.x, m_Vec.y, m_Vec.z, m_Ang.z, m_Ang.x, m_Ang.y);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		outVector = m_Vec;
		outAngles = m_Ang;

		return true;
	}


	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("x", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.x = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s x <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.x
				);
				return;
			}
			else if (0 == _stricmp("y", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.y = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s y <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.y
				);
				return;
			}
			else if (0 == _stricmp("z", arg1))
			{
				if (3 <= argc)
				{
					m_Vec.z = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s z <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Vec.z
				);
				return;
			}
			else if (0 == _stricmp("rX", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.z = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rX <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.z
				);
				return;
			}
			else if (0 == _stricmp("rY", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.x = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rY <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.x
				);
				return;
			}
			else if (0 == _stricmp("rZ", arg1))
			{
				if (3 <= argc)
				{
					m_Ang.y = (float)atof(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s rZ <fValue> - Set new value.\n"
					"Current value: %f\n"
					, arg0
					, m_Ang.y
				);
				return;
			}
		}

		Tier0_Msg(
			"%s x [...]\n"
			"%s y [...]\n"
			"%s z [...]\n"
			"%s rX [...]\n"
			"%s rY [...]\n"
			"%s rZ [...]\n"
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
			, arg0
		);
	}

private:
	SOURCESDK::Vector m_Vec;
	SOURCESDK::QAngle m_Ang;
};


class CMirvVecAngOffsetCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngOffsetCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset, bool local)
		: CMirvVecAngCalc(name)
		, m_Parent(parent)
		, m_Offset(offset)
		, m_Local(local)
	{
		m_Parent->AddRef();
		m_Offset->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=offset parent=\"%s\" offset=\"%s\" local=%i", m_Parent->GetName(), m_Offset->GetName(), m_Local ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("local", arg1))
			{
				if (3 <= argc)
				{
					m_Local = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s local <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_Local ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s local [...]\n"
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::Vector parentVector;
		SOURCESDK::QAngle parentAngles;
		SOURCESDK::Vector offsetVector;
		SOURCESDK::QAngle offsetAngles;
		bool calcedParent = m_Parent->CalcVecAng(parentVector, parentAngles);
		bool calcedOffset = calcedParent && m_Offset->CalcVecAng(offsetVector, offsetAngles);

		if (calcedParent && calcedOffset)
		{
			if (m_Local)
			{
				if (offsetVector.x || offsetVector.y || offsetVector.z)
				{
					double forward[3], right[3], up[3];

					Afx::Math::MakeVectors(parentAngles.z, parentAngles.x, parentAngles.y, forward, right, up);

					outVector.x = (float)(parentVector.x + offsetVector.x*forward[0] - offsetVector.y*right[0] + offsetVector.z*up[0]);
					outVector.y = (float)(parentVector.y + offsetVector.x*forward[1] - offsetVector.y*right[1] + offsetVector.z*up[1]);
					outVector.z = (float)(parentVector.z + offsetVector.x*forward[2] - offsetVector.y*right[2] + offsetVector.z*up[2]);
				}
				else
				{
					outVector = parentVector;
				}

				if (offsetAngles.x || offsetAngles.y || offsetAngles.z)
				{
					Afx::Math::Quaternion q1 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(parentAngles.x, parentAngles.y, parentAngles.z)));
					Afx::Math::Quaternion q2 = Afx::Math::Quaternion::FromQREulerAngles(Afx::Math::QREulerAngles::FromQEulerAngles(Afx::Math::QEulerAngles(offsetAngles.x, offsetAngles.y, offsetAngles.z)));

					Afx::Math::QEulerAngles angs = (q1 * q2).ToQREulerAngles().ToQEulerAngles();

					outAngles.z = (float)angs.Roll;
					outAngles.x = (float)angs.Pitch;
					outAngles.y = (float)angs.Yaw;
				}
				else
				{
					outAngles = parentAngles;
				}
			}
			else
			{
				outVector.x = parentVector.x + offsetVector.x;
				outVector.y = parentVector.y + offsetVector.y;
				outVector.z = parentVector.z + offsetVector.z;
				outAngles.x = parentAngles.x + offsetAngles.x;
				outAngles.y = parentAngles.y + offsetAngles.y;
				outAngles.z = parentAngles.z + offsetAngles.z;
			}
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngOffsetCalc()
	{
		m_Offset->Release();
		m_Parent->Release();
	}

private:
	IMirvVecAngCalc * m_Parent;
	IMirvVecAngCalc * m_Offset;
	bool m_Local;
};

class CMirvVecAngHandleCalcEx : public CMirvVecAngCalc
{
public:
	CMirvVecAngHandleCalcEx(char const * name, IMirvHandleCalc * handle, bool eyeVec, bool eyeAng)
		: CMirvVecAngCalc(name)
		, m_Handle(handle)
		, m_EyeVec(eyeVec)
		, m_EyeAng(eyeAng)
	{
		m_Handle->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=handleEx handle=\"%s\" eyeVec=%i, eyeAng=%i", m_Handle->GetName(), m_EyeVec ? 1 : 0, m_EyeAng ? 1 : 0);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("eyeVec", arg1))
			{
				if (3 <= argc)
				{
					m_EyeVec = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s eyeVec <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_EyeVec ? 1 : 0
				);
				return;
			}
			else if (0 == _stricmp("eyeAng", arg1))
			{
				if (3 <= argc)
				{
					m_EyeAng = atoi(args->ArgV(2));
					return;
				}

				Tier0_Msg(
					"%s eyeAng <bValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_EyeAng ? 1 : 0
				);
				return;
			}
		}

		Tier0_Msg(
			"%s eyeVec [...]\n"
			"%s eyeAng [...]\n"
			, arg0
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		bool calcedHandle = m_Handle->CalcHandle(handle);
		SOURCESDK::IClientEntity_csgo * ce = calcedHandle ? SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle) : 0;
		SOURCESDK::C_BaseEntity_csgo * be = ce ? ce->GetBaseEntity() : 0;

		if (be || ce && !m_EyeVec && m_EyeAng)
		{
			outVector = m_EyeVec ? be->EyePosition() : ce->GetAbsOrigin();
			outAngles = m_EyeAng ? be->EyeAngles() : ce->GetAbsAngles();
			return true;
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngHandleCalcEx()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
	bool m_EyeVec;
	bool m_EyeAng;
};

class CMirvVecAngHandleAttachmentCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngHandleAttachmentCalc(char const * name, IMirvHandleCalc * handle, char const * attachmentName)
		: CMirvVecAngCalc(name)
		, m_Handle(handle)
		, m_AttachmentName(attachmentName)
	{
		m_Handle->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=handleAttachment handle=\"%s\" attachmentName=\"%s\"", m_Handle->GetName(), m_AttachmentName.c_str());
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("attachmentName", arg1))
			{
				if (3 <= argc)
				{
					m_AttachmentName = args->ArgV(2);
					return;
				}

				Tier0_Msg(
					"%s attachmentName <sValue> - Set new value.\n"
					"Current value: %i\n"
					, arg0
					, m_AttachmentName.c_str()
				);
				return;
			}
		}

		Tier0_Msg(
			"%s attachmentName [...]\n"
			, arg0
		);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		SOURCESDK::CSGO::CBaseHandle handle;
		bool calcedHandle = m_Handle->CalcHandle(handle);
		SOURCESDK::IClientEntity_csgo * ce = calcedHandle ? SOURCESDK::g_Entitylist_csgo->GetClientEntityFromHandle(handle) : 0;

		if (ce)
		{
			int idx = ce->LookupAttachment(m_AttachmentName.c_str());
			if (-1 != idx)
			{
				return ce->GetAttachment(idx, outVector, outAngles);
			}
		}

		return false;
	}

protected:
	virtual ~CMirvVecAngHandleAttachmentCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
	std::string m_AttachmentName;
};

class CMirvVecAngIfCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngIfCalc(char const * name, IMirvBoolCalc * condition, IMirvVecAngCalc * condTrue, IMirvVecAngCalc * condFalse)
		: CMirvVecAngCalc(name)
		, m_Condition(condition)
		, m_CondTrue(condTrue)
		, m_CondFalse(condFalse)
	{
		m_Condition->AddRef();
		m_CondTrue->AddRef();
		m_CondFalse->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=if condition=\"%s\" true=\"%s\" false=\"%s\"", m_Condition->GetName(), m_CondTrue->GetName(), m_CondFalse->GetName());
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		bool condition = m_Condition->CalcBool();

		return condition ? m_CondTrue->CalcVecAng(outVector, outAngles) : m_CondFalse->CalcVecAng(outVector, outAngles);
	}

protected:
	virtual ~CMirvVecAngIfCalc()
	{
		m_CondFalse->Release();
		m_CondTrue->Release();
		m_Condition->Release();
	}

private:
	IMirvBoolCalc * m_Condition;
	IMirvVecAngCalc * m_CondTrue;
	IMirvVecAngCalc * m_CondFalse;
};


class CMirvVecAngOrCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngOrCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
		: CMirvVecAngCalc(name)
		, m_A(a)
		, m_B(b)
	{
		m_A->AddRef();
		m_B->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=or a=\"%s\" b=\"%s\"", m_A->GetName(), m_B->GetName());
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		return m_A->CalcVecAng(outVector, outAngles) || m_B->CalcVecAng(outVector, outAngles);
	}

protected:
	virtual ~CMirvVecAngOrCalc()
	{
		m_B->Release();
		m_A->Release();
	}

private:
	IMirvVecAngCalc * m_A;
	IMirvVecAngCalc * m_B;
};

class CMirvCamCamCalc : public CMirvCamCalc
{
public:
	CMirvCamCamCalc(char const * name, const char * camFileName, const char * startClientTime)
		: CMirvCamCalc(name)
		, m_CamFileName(camFileName)
		, m_StartClientTime(StartClientTimeFromString(startClientTime))
	{
		m_CamImport = new CamImport(m_CamFileName.c_str(), m_StartClientTime);
	}

	virtual void Console_Print(void)
	{
		CMirvCamCalc::Console_Print();

		Tier0_Msg(" type=cam camFileName=\"%s\" startClientTime=%f", m_CamFileName.c_str(), m_StartClientTime);
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		int argc = args->ArgC();
		char const * arg0 = args->ArgV(0);

		if (2 <= argc)
		{
			char const * arg1 = args->ArgV(1);

			if (0 == _stricmp("filePath", arg1))
			{
				if (3 <= argc)
				{
					m_CamFileName = args->ArgV(2);
					delete m_CamImport;
					m_CamImport = new CamImport(m_CamFileName.c_str(), m_StartClientTime);
					if(m_CamImport->IsBad()) Tier0_Warning("Error importing CAM file \"%s\"\n", m_CamFileName.c_str());
					return;
				}

				Tier0_Msg(
					"%s filePath <sFilePath> - Set mirv_camio input file name / path.\n"
					"Current value: %s\n"
					, arg0
					, m_CamFileName.c_str()
				);
				return;
			}
			else if (0 == _stricmp("startTime", arg1))
			{
				if (3 <= argc)
				{
					m_StartClientTime = StartClientTimeFromString(args->ArgV(2));
					m_CamImport->SetStart(m_StartClientTime);
					return;
				}

				Tier0_Msg(
					"%s startTime <fStartTime>|current - Set mirv_camio input file name / path.\n"
					"Current value: %f\n"
					, arg0
					, m_StartClientTime
				);
				return;
			}
		}

		Tier0_Msg(
			"%s filePath [...]\n"
			"%s startTime [...]\n"
			, arg0
			, arg0
		);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		CamIO::CamData outCamData;

		if (m_CamImport->GetCamData(g_Hook_VClient_RenderView.GetCurTime(), g_Hook_VClient_RenderView.LastWidth, g_Hook_VClient_RenderView.LastHeight, outCamData))
		{
			outVector.x = (float)outCamData.XPosition;
			outVector.y = (float)outCamData.YPosition;
			outVector.z = (float)outCamData.ZPosition;
			outAngles.x = (float)outCamData.YRotation;
			outAngles.y = (float)outCamData.ZRotation;
			outAngles.z = (float)outCamData.XRotation;
			outFov = (float)outCamData.Fov;

			return true;
		}

		return false;
	}

	virtual bool IsBad() {
		return m_CamImport->IsBad();
	}

protected:
	virtual ~CMirvCamCamCalc()
	{
		delete m_CamImport;
	}

private:
	CamImport * m_CamImport;
	std::string m_CamFileName;
	double m_StartClientTime;

	double StartClientTimeFromString(const char * startClientTime)
	{
		return 0 == _stricmp(startClientTime, "current") ? g_Hook_VClient_RenderView.GetCurTime() : atof(startClientTime);
	}
};

class CMirvCamGameCalc : public CMirvCamCalc
{
public:
	CMirvCamGameCalc(char const * name)
		: CMirvCamCalc(name)
	{
	}

	virtual void Console_Print(void)
	{
		CMirvCamCalc::Console_Print();

		Tier0_Msg(" type=game");
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCamCalc::Console_Edit(args);
	}

	virtual bool CalcCam(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles, float & outFov)
	{
		outVector.x = g_Hook_VClient_RenderView.GameCameraOrigin[0];
		outVector.y = g_Hook_VClient_RenderView.GameCameraOrigin[1];
		outVector.z = g_Hook_VClient_RenderView.GameCameraOrigin[2];
		outAngles.x = g_Hook_VClient_RenderView.GameCameraAngles[0];
		outAngles.y = g_Hook_VClient_RenderView.GameCameraAngles[1];
		outAngles.z = g_Hook_VClient_RenderView.GameCameraAngles[2];
		outFov = g_Hook_VClient_RenderView.GameCameraFov;

		return true;
	}

protected:
	virtual ~CMirvCamGameCalc()
	{
	}

private:

};

class CMirvVecAngCamCalc : public CMirvVecAngCalc
{
public:
	CMirvVecAngCamCalc(char const * name, IMirvCamCalc * cam)
		: CMirvVecAngCalc(name)
		, m_Cam(cam)
	{
		m_Cam->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvVecAngCalc::Console_Print();

		Tier0_Msg(" type=cam cam=\"%s\"", m_Cam->GetName());
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvVecAngCalc::Console_Edit(args);
	}

	virtual bool CalcVecAng(SOURCESDK::Vector & outVector, SOURCESDK::QAngle & outAngles)
	{
		float dummyFov;

		return m_Cam->CalcCam(outVector, outAngles, dummyFov);
	}

protected:
	virtual ~CMirvVecAngCamCalc()
	{
		m_Cam->Release();
	}

private:
	IMirvCamCalc * m_Cam;
};

class CMirvFovCamCalc : public CMirvFovCalc
{
public:
	CMirvFovCamCalc(char const * name, IMirvCamCalc * cam)
		: CMirvFovCalc(name)
		, m_Cam(cam)
	{
		m_Cam->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvFovCalc::Console_Print();

		Tier0_Msg(" type=cam cam=\"%s\"", m_Cam->GetName());
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvFovCalc::Console_Edit(args);
	}

	virtual bool CalcFov(float & outFov)
	{
		SOURCESDK::Vector dummyVector;
		SOURCESDK::QAngle dummyAngles;

		return m_Cam->CalcCam(dummyVector, dummyAngles, outFov);
	}

protected:
	virtual ~CMirvFovCamCalc()
	{
		m_Cam->Release();
	}

private:
	IMirvCamCalc * m_Cam;
};

class CMirvBoolCalc : public CMirvCalc, public IMirvBoolCalc
{
public:
	CMirvBoolCalc(char const * name)
		: CMirvCalc(name)
	{

	}

	virtual void AddRef(void)
	{
		CMirvCalc::AddRef();
	}

	virtual void Release(void)
	{
		CMirvCalc::Release();
	}

	virtual int GetRefCount(void)
	{
		return CMirvCalc::GetRefCount();
	}

	virtual  char const * GetName(void)
	{
		return CMirvCalc::GetName();
	}

	virtual void Console_Print(void)
	{
		CMirvCalc::Console_Print();
	}

	virtual bool CalcBool(void)
	{
		return false;
	}

	virtual void Console_Edit(IWrpCommandArgs * args)
	{
		CMirvCalc::Console_Edit(args);
	}
};

class CMirvBoolHandleCalc : public CMirvBoolCalc
{
public:
	CMirvBoolHandleCalc(char const * name, IMirvHandleCalc * handle)
		: CMirvBoolCalc(name)
		, m_Handle(handle)
	{
		m_Handle->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvBoolCalc::Console_Print();

		Tier0_Msg(" type=handle handle=\"%s\"", m_Handle->GetName());
	}

	virtual bool CalcBool(void)
	{
		SOURCESDK::CSGO::CBaseHandle dummy;

		return m_Handle->CalcHandle(dummy);
	}

protected:
	virtual ~CMirvBoolHandleCalc()
	{
		m_Handle->Release();
	}

private:
	IMirvHandleCalc * m_Handle;
};

class CMirvBoolVecAngCalc : public CMirvBoolCalc
{
public:
	CMirvBoolVecAngCalc(char const * name, IMirvVecAngCalc * vecAng)
		: CMirvBoolCalc(name)
		, m_VecAng(vecAng)
	{
		m_VecAng->AddRef();
	}

	virtual void Console_Print(void)
	{
		CMirvBoolCalc::Console_Print();

		Tier0_Msg(" type=vecAng vecAng=\"%s\"", m_VecAng->GetName());
	}

	virtual bool CalcBool(void)
	{
		SOURCESDK::Vector dummyVec;
		SOURCESDK::QAngle dummyAng;

		return m_VecAng->CalcVecAng(dummyVec, dummyAng);
	}

protected:
	virtual ~CMirvBoolVecAngCalc()
	{
		m_VecAng->Release();
	}

private:
	IMirvVecAngCalc * m_VecAng;
};


CMirvHandleCalcs::~CMirvHandleCalcs()
{
	for (std::list<IMirvHandleCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvHandleCalc * CMirvHandleCalcs::GetByName(char const * name)
{
	std::list<IMirvHandleCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvHandleCalc * CMirvHandleCalcs::NewValueCalc(char const * name, int handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleValueCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc * CMirvHandleCalcs::NewIndexCalc(char const * name, int entityIndex)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleIndexCalc(name, entityIndex);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc * CMirvHandleCalcs::NewActiveWeaponCalc(char const * name, IMirvHandleCalc * parent, bool world)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleActiveWeaponCalc(name, parent, world);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvHandleCalc * CMirvHandleCalcs::NewKeyCalc(char const * name, int key)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvHandleCalc * result = new CMirvHandleKeyCalc(name, key);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvHandleCalcs::Console_Remove(char const * name)
{
	std::list<IMirvHandleCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvHandleCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvHandleCalcs::Console_Print(void)
{
	for (std::list<IMirvHandleCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_Print(); Tier0_Msg(";\n");
	}
}

void CMirvHandleCalcs::GetIteratorByName(char const * name, std::list<IMirvHandleCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvVecAngCalcs::~CMirvVecAngCalcs()
{
	for (std::list<IMirvVecAngCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvVecAngCalc * CMirvVecAngCalcs::GetByName(char const * name)
{
	std::list<IMirvVecAngCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewValueCalc(char const * name, float x, float y, float z, float rX, float rY, float rZ)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngValueCalc(name, x, y, z, rX, rY, rZ);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewOffsetCalc(char const * name, IMirvVecAngCalc * parent, IMirvVecAngCalc * offset, bool local)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngOffsetCalc(name, parent, offset, local);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, false, false);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleEyeCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, true, true);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleCalcEx(char const * name, IMirvHandleCalc * handle, bool eyeVec, bool eyeAng)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleCalcEx(name, handle, eyeVec, eyeAng);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewHandleAttachmentCalc(char const * name, IMirvHandleCalc * handle, char const * attachmentName)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngHandleAttachmentCalc(name, handle, attachmentName);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewIfCalc(char const * name, IMirvBoolCalc * condition, IMirvVecAngCalc * condTrue, IMirvVecAngCalc * condFalse)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngIfCalc(name, condition, condTrue, condFalse);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewOrCalc(char const * name, IMirvVecAngCalc * a, IMirvVecAngCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngOrCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvVecAngCalc * CMirvVecAngCalcs::NewCamCalc(char const * name, IMirvCamCalc * src)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvVecAngCalc * result = new CMirvVecAngCamCalc(name, src);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvVecAngCalcs::Console_Remove(char const * name)
{
	std::list<IMirvVecAngCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvVecAngCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvVecAngCalcs::Console_Print(void)
{
	for (std::list<IMirvVecAngCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_Print(); Tier0_Msg(";\n");
	}
}

void CMirvVecAngCalcs::GetIteratorByName(char const * name, std::list<IMirvVecAngCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}



CMirvCamCalcs::~CMirvCamCalcs()
{
	for (std::list<IMirvCamCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvCamCalc * CMirvCamCalcs::GetByName(char const * name)
{
	std::list<IMirvCamCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvCamCalc * CMirvCamCalcs::NewCamCalc(char const * name, const char * camFileName, const char * startClientTime)
{
	if (name && !Console_CheckName(name))
		return 0;

	CMirvCamCamCalc * result = new CMirvCamCamCalc(name, camFileName, startClientTime);

	if(result->IsBad()) Tier0_Warning("Error importing CAM file \"%s\"\n", name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvCamCalc * CMirvCamCalcs::NewGameCalc(char const * name)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvCamCalc * result = new CMirvCamGameCalc(name);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}


void CMirvCamCalcs::Console_Remove(char const * name)
{
	std::list<IMirvCamCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvCamCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvCamCalcs::Console_Print(void)
{
	for (std::list<IMirvCamCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_Print(); Tier0_Msg(";\n");
	}
}

void CMirvCamCalcs::GetIteratorByName(char const * name, std::list<IMirvCamCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvFovCalcs::~CMirvFovCalcs()
{
	for (std::list<IMirvFovCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvFovCalc * CMirvFovCalcs::GetByName(char const * name)
{
	std::list<IMirvFovCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvFovCalc * CMirvFovCalcs::NewCamCalc(char const * name, IMirvCamCalc * src)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvFovCalc * result = new CMirvFovCamCalc(name, src);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

void CMirvFovCalcs::Console_Remove(char const * name)
{
	std::list<IMirvFovCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvFovCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvFovCalcs::Console_Print(void)
{
	for (std::list<IMirvFovCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_Print(); Tier0_Msg(";\n");
	}
}

void CMirvFovCalcs::GetIteratorByName(char const * name, std::list<IMirvFovCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}


CMirvBoolCalcs::~CMirvBoolCalcs()
{
	for (std::list<IMirvBoolCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Release();
	}
}

IMirvBoolCalc * CMirvBoolCalcs::GetByName(char const * name)
{
	std::list<IMirvBoolCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		return *it;
	}

	return 0;
}

IMirvBoolCalc * CMirvBoolCalcs::NewHandleCalc(char const * name, IMirvHandleCalc * handle)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolHandleCalc(name, handle);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewVecAngCalc(char const * name, IMirvVecAngCalc * vecAng)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolVecAngCalc(name, vecAng);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

/*

IMirvBoolCalc * CMirvBoolCalcs::NewAndCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolAndCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewOrCalc(char const * name, IMirvBoolCalc * a, IMirvBoolCalc * b)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolOrCalc(name, a, b);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

IMirvBoolCalc * CMirvBoolCalcs::NewNotCalc(char const * name, IMirvBoolCalc * a)
{
	if (name && !Console_CheckName(name))
		return 0;

	IMirvBoolCalc * result = new CMirvBoolNotCalc(name, a);

	if (name)
	{
		result->AddRef();

		m_Calcs.push_back(result);
	}

	return result;
}

*/

void CMirvBoolCalcs::Console_Remove(char const * name)
{
	std::list<IMirvBoolCalc *>::iterator it;
	GetIteratorByName(name, it);
	if (it != m_Calcs.end())
	{
		if (1 == (*it)->GetRefCount())
		{
			(*it)->Release();
			m_Calcs.erase(it);
		}
		else
			Tier0_Warning("Error: Cannot remove %s: Still in use.\n", (*it)->GetName());
	}
	else
	{
		Tier0_Warning("Error: No Calc named \"%s\" found.\n", name);
	}
}

bool CMirvBoolCalcs::Console_CheckName(char const * name)
{
	if (!name)
	{
		Tier0_Warning("Error: Name cannot be null pointer.\n");
		return false;
	}

	if (!isalpha(*name))
	{
		Tier0_Warning("Error: Name has to begin with an alphabet letter.\n");
		return false;
	}

	if (!StringIsAlNum(name))
	{
		Tier0_Warning("Error: Name has to be alpha-numeric (letters and digits).\n");
		return false;
	}

	if (GetByName(name))
	{
		Tier0_Warning("Error: Name is already in use.\n");
		return false;
	}

	return true;
}

void CMirvBoolCalcs::Console_Print(void)
{
	for (std::list<IMirvBoolCalc *>::iterator it = m_Calcs.begin(); it != m_Calcs.end(); ++it)
	{
		(*it)->Console_Print(); Tier0_Msg(";\n");
	}
}

void CMirvBoolCalcs::GetIteratorByName(char const * name, std::list<IMirvBoolCalc *>::iterator & outIt)
{
	for (outIt = m_Calcs.begin(); outIt != m_Calcs.end(); ++outIt)
	{
		if (0 == _stricmp(name, (*outIt)->GetName()))
			break;
	}
}

void mirv_calcs_handle(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewValueCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("index", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewIndexCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("key", arg2) && 5 <= argc)
				{
					g_MirvHandleCalcs.NewKeyCalc(args->ArgV(3), atoi(args->ArgV(4)));
					return;
				}
				else if (0 == _stricmp("activeWeapon", arg2) && 6 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvHandleCalcs.NewActiveWeaponCalc(args->ArgV(3), parentCalc, 0 != atoi(args->ArgV(5)));
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <iHandle> - Add a new calc with a constant value.\n"
				"%s add index <sName> <iIndex> - Add a new index calc.\n"
				"%s add key <sName> <iKeyNumber> - Add a new key calc (like spectator HUD).\n"
				"%s add activeWeapon <sName> <sParentCalcHandleName> <bGetWorld> - Add an active weapon calc, <bGetWorld> is 0 or 1.\n"
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvHandleCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvHandleCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::CSGO::CBaseHandle handle;
				bool calced = parentCalc->CalcHandle(handle);

				Tier0_Msg("Calc: ");
				parentCalc->Console_Print();
				if(calced)
					Tier0_Msg("\nResult: true, handle=%i\n", handle);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new handle calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

void mirv_calcs_vecang(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("value", arg2) && 10 <= argc)
				{
					g_MirvVecAngCalcs.NewValueCalc(args->ArgV(3), (float)atof(args->ArgV(4)), (float)atof(args->ArgV(5)), (float)atof(args->ArgV(6)), (float)atof(args->ArgV(7)), (float)atof(args->ArgV(8)), (float)atof(args->ArgV(9)));
					return;
				}
				else if (0 == _stricmp("offset", arg2) && 7 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewOffsetCalc(args->ArgV(3), calcA, calcB, 0 != atoi(args->ArgV(6)));
						}
						else
							Tier0_Warning("Error: No handle parent with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No handle offset with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("handle", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handleEye", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleEyeCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("handleAttachment", arg2) && 6 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvHandleCalc * parentCalc = g_MirvHandleCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewHandleAttachmentCalc(args->ArgV(3), parentCalc, args->ArgV(5));
					else
						Tier0_Warning("Error: No handle calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
				else if (0 == _stricmp("or", arg2) && 6 <= argc)
				{
					char const * calcAName = args->ArgV(4);
					IMirvVecAngCalc * calcA = g_MirvVecAngCalcs.GetByName(calcAName);

					if (calcA)
					{
						char const * calcBName = args->ArgV(5);
						IMirvVecAngCalc * calcB = g_MirvVecAngCalcs.GetByName(calcBName);

						if (calcB)
						{
							g_MirvVecAngCalcs.NewOrCalc(args->ArgV(3), calcA, calcB);
						}
						else
							Tier0_Warning("Error: No handle calcB with name \"%s\" found.\n", calcBName);
					}
					else
						Tier0_Warning("Error: No handle calcA with name \"%s\" found.\n", calcAName);

					return;
				}
				else if (0 == _stricmp("cam", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvVecAngCalcs.NewCamCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No cam calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add value <sName> <fX> <fY> <fZ> <rX> <rY> <rZ> - Add a new calc with a constant value.\n"
				"%s add offset <sName> <sParentName> <sOffSetName> <bIsLocal> - Add a new offset calc, <bIsLocal> is 1 for local transform, 0 for global transform.\n"
				"%s add handle <sName> <sHandleCalcName> - Add an calc that gets its values from an entity using a handle calc named <sHandleCalcName>.\n"
				"%s add handleEye <sName> <sHandleCalcName> - Add an calc that gets its values from an entity's eye point using a handle calc named <sHandleCalcName>.\n"
				"%s add handleAttachment <sName> <sHandleCalcName> <sAttachMentName> - Add an calc that gets its values from an entity's attachment.\n"
				"%s add or <sName> <sAName> <sBName> - Add an OR calc.\n"
				"%s add cam <sName> <sCamCalName> - Adds a calc that gets its values from an cam calc named <sCamCalName>.\n"
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvVecAngCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvVecAngCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvVecAngCalc * parentCalc = g_MirvVecAngCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::Vector vec;
				SOURCESDK::QAngle ang;
				bool calced = parentCalc->CalcVecAng(vec, ang);

				Tier0_Msg("Calc: ");
				parentCalc->Console_Print();
				if(calced)
					Tier0_Msg("\nResult: true, vec=(%f, %f, %f), ang=(%f, %f, %f)\n", vec.x, vec.y, vec.z, ang.z, ang.x, ang.y);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvVecAngCalc * parentCalc = g_MirvVecAngCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


void mirv_calcs_cam(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("cam", arg2) && 6 <= argc)
				{
					char const * camFileName = args->ArgV(4);
					char const * szStartClientTime = args->ArgV(5);

					g_MirvCamCalcs.NewCamCalc(args->ArgV(3), camFileName, szStartClientTime);

					return;
				}
				else if (0 == _stricmp("game", arg2))
				{
					g_MirvCamCalcs.NewGameCalc(args->ArgV(3));

					return;
				}
			}

			Tier0_Msg(
				"%s add cam <sName> <sfilePath> <fStartTime>|current - Adds an mirv_camio file as calc.\n"
				"%s add game <sName> - Current game camera.\n"
				, arg0
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvCamCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvCamCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				SOURCESDK::Vector vec;
				SOURCESDK::QAngle ang;
				float fov;
				bool calced = parentCalc->CalcCam(vec, ang, fov);

				Tier0_Msg("Calc: ");
				parentCalc->Console_Print();
				if (calced)
					Tier0_Msg("\nResult: true, vec=(%f, %f, %f), ang=(%f, %f, %f), fov=%f\n", vec.x, vec.y, vec.z, ang.z, ang.x, ang.y, fov);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}



void mirv_calcs_fov(IWrpCommandArgs * args)
{
	int argc = args->ArgC();

	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("add", arg1))
		{
			if (3 <= argc)
			{
				char const * arg2 = args->ArgV(2);

				if (0 == _stricmp("cam", arg2) && 5 <= argc)
				{
					char const * parentCalcName = args->ArgV(4);

					IMirvCamCalc * parentCalc = g_MirvCamCalcs.GetByName(parentCalcName);

					if (parentCalc)
						g_MirvFovCalcs.NewCamCalc(args->ArgV(3), parentCalc);
					else
						Tier0_Warning("Error: No cam calc with name \"%s\" found.\n", parentCalcName);

					return;
				}
			}

			Tier0_Msg(
				"%s add cam <sName> <sCamCalName> - Adds a calc that gets its values from an cam calc named <sCamCalName>.\n"
				, arg0
			);
			return;
		}
		else if (0 == _stricmp("remove", arg1) && 3 <= argc)
		{
			g_MirvFovCalcs.Console_Remove(args->ArgV(2));
			return;
		}
		else if (0 == _stricmp("print", arg1))
		{
			g_MirvFovCalcs.Console_Print();
			return;
		}
		else if (0 == _stricmp("test", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvFovCalc * parentCalc = g_MirvFovCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				float fov;
				bool calced = parentCalc->CalcFov(fov);

				Tier0_Msg("Calc: ");
				parentCalc->Console_Print();
				if (calced)
					Tier0_Msg("\nResult: true, fov=%f\n", fov);
				else
					Tier0_Msg("\nResult: false\n");
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
		else if (0 == _stricmp("edit", arg1) && 3 <= argc)
		{
			char const * parentCalcName = args->ArgV(2);

			IMirvFovCalc * parentCalc = g_MirvFovCalcs.GetByName(parentCalcName);

			if (parentCalc)
			{
				CSubWrpCommandArgs sub(args, 3);
				parentCalc->Console_Edit(&sub);
				return;
			}
			else
				Tier0_Warning("Error: No calc with name \"%s\" found.\n", parentCalcName);

			return;
		}
	}

	Tier0_Msg(
		"%s add [...] - Add a new calc.\n"
		"%s remove <sCalcName> - Remove calc with name <sCalcName>.\n"
		"%s print - Print calcs.\n"
		"%s test <sCalcName> - Test a calc.\n"
		"%s edit <sCalcName> [...] - Edit a calc.\n"
		, arg0
		, arg0
		, arg0
		, arg0
		, arg0
	);
}


CON_COMMAND(mirv_calcs, "Expressions, currently mainly for usage mirv_calcs, mirv_cam, mirv_aim")
{
	int argc = args->ArgC();
	char const * arg0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * arg1 = args->ArgV(1);

		if (0 == _stricmp("handle", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_handle(&sub);
			return;
		}
		else if (0 == _stricmp("vecAng", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_vecang(&sub);
			return;
		}
		else if (0 == _stricmp("fov", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_fov(&sub);
			return;
		}
		else if (0 == _stricmp("cam", arg1))
		{
			CSubWrpCommandArgs sub(args, 2);
			mirv_calcs_cam(&sub);
			return;
		}
	}

	Tier0_Msg(
		"%s handle [...] - Calcs that return an entity handle.\n"
		"%s vecAng [...] - Calcs that return VecAng (location and rotation).\n"
		"%s fov [...] - Calcs that return FOV (field of view).\n"
		"%s cam [...] - Calcs that return a view (location, rotation and FOV).\n"
		, arg0
		, arg0
		, arg0
		, arg0
	);
}

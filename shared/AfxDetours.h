#pragma once

#include <Windows.h>
#include <shared/Detours/src/detours.h>
#include <vector>
#include <map>
#include <string>

typedef struct MdtMemBlockInfo_s {
	PVOID  BaseAddress;
	SIZE_T RegionSize;
	DWORD  Protect;
} MdtMemBlockInfo;

typedef std::vector<MdtMemBlockInfo> MdtMemBlockInfos;

/// <remarks> len must be at least 5 </remarks>
void *DetourApply(BYTE *orig, BYTE *hook, int len);

/// <remarks> len must be at least 8, (class pointer in ecx is pushed over stack).
///  this one destroys the value of eax, if that is a problem then use
///  DetourVoidClassFunc instead.
/// </remarks>
void *DetourClassFunc(BYTE *src, const BYTE *dst, const int len);


typedef void(*DetourIfacePtr_fn)(void);

/// <remarks>This is somewhat threadsafe, but be aware that outTarget can be already called while you are still in this function.</remarks>
void DetourIfacePtr(DWORD * ptr, void const * hookk, DetourIfacePtr_fn & outTarget);

void MdtMemAccessBegin(LPVOID lpAddress, size_t size, MdtMemBlockInfos *mdtMemBlockInfos);
void MdtMemAccessEnd(MdtMemBlockInfos *mdtMemBlockInfos);
LPVOID MdtAllocExecuteableMemory(size_t size);

/// <summary> Replaces a block of 32 bit x86 code with a JMP instruction. </summary>
/// <remarks> countBytes must be at least 5. </remarks>
void Asm32ReplaceWithJmp(void * replaceAt, size_t countBytes, void * jmpTo);


class CAfxImportFuncHookBase
{
public:
	CAfxImportFuncHookBase(bool useOrdinal, DWORD ordinal, LPCSTR name)
		: m_UseOrdinal(useOrdinal)
		, m_Ordinal(ordinal)
		, m_Name(name)
	{
	}

	BOOL Apply(PVOID* ppvFunc);
	BOOL Unapply(PVOID* ppvFunc);

	bool GetUseOrdinal()
	{
		return m_UseOrdinal;
	}

	DWORD GetOrdinal()
	{
		return m_Ordinal;
	}

	LPCSTR GetName()
	{
		return m_Name;
	}

	virtual PVOID* GetMyFunc() = 0;
	virtual PVOID* GetTrueFunc() = 0;

private:
	bool m_UseOrdinal;
	DWORD m_Ordinal;
	LPCSTR m_Name;
};


template <class T> class CAfxImportFuncHook : public CAfxImportFuncHookBase
{
public:
	T MyFunc;
	T TrueFunc = nullptr;

	typename CAfxImportFuncHook(LPCSTR name, T myFunc)
		: CAfxImportFuncHookBase(false, (DWORD)-1, name)
		, MyFunc(myFunc)
	{
	}

	CAfxImportFuncHook(DWORD ordinal, PVOID myFunc)
		: MyFunc(myFunc)
		, m_UseOrdinal(true)
		, m_Ordinal(ordinal)
		, m_Name(nullptr)
	{
	}

	CAfxImportFuncHook(DWORD ordinal, LPCSTR name, PVOID myFunc)
		: MyFunc(myFunc)
		, m_UseOrdinal(true)
		, m_Ordinal(ordinal)
		, m_Name(name)
	{
	}

	bool GetUseOrdinal()
	{
		return m_UseOrdinal;
	}

	DWORD GetOrdinal()
	{
		return m_Ordinal;
	}

	LPCSTR GetName()
	{
		return m_Name;
	}

	virtual PVOID* GetMyFunc()
	{
		return (PVOID*)&MyFunc;
	}

	virtual PVOID* GetTrueFunc()
	{
		return (PVOID*)&TrueFunc;
	}
};

typedef std::vector<CAfxImportFuncHookBase*> CAfxImportDllHooks;

class CAfxImportDllHook
{
public:
	CAfxImportDllHook(LPCSTR name)
		: m_Name(name)
	{

	}

	CAfxImportDllHook(LPCSTR name, const CAfxImportDllHooks& hooks)
		: m_Name(name)
	{
		Add(hooks);
	}

	void Add(const CAfxImportDllHooks& hooks)
	{
		for (std::vector<CAfxImportFuncHookBase*>::const_iterator it = hooks.begin(); it != hooks.end(); ++it)
		{
			CAfxImportFuncHookBase* hook = *it;

			if (hook->GetUseOrdinal())
			{
				if (hook->GetUseOrdinal() && hook->GetName())
				{
					m_FuncHooksByOrdinalAndName[CAfxDllImportFuncOrdinalAndName(hook->GetOrdinal(), hook->GetName())] = hook;
				}
				else
				{
					m_FuncHooksByOrdinal[CAfxDllImportFuncOrdinal(hook->GetOrdinal())] = hook;
				}
			}
			else
			{
				m_FuncHooksByName[CAfxDllImportFuncName(hook->GetName())] = hook;
			}
		}
	}

	BOOL Apply(DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc);
	BOOL Unapply(DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc);

	LPCSTR GetName() const
	{
		return m_Name;
	}

private:
	class CAfxDllImportFuncName
	{
	public:
		CAfxDllImportFuncName(LPCSTR name)
			: m_Name(name)
		{
		}

		bool operator<(const CAfxDllImportFuncName& y) const {
			return strcmp(m_Name, y.m_Name) < 0;
		}

	private:
		LPCSTR m_Name;
	};

	class CAfxDllImportFuncOrdinal
	{
	public:
		CAfxDllImportFuncOrdinal(DWORD ordinal)
		{
		}

		bool operator<(const CAfxDllImportFuncOrdinal& y) const {
			return m_Ordinal < y.m_Ordinal;
		}


	private:
		DWORD m_Ordinal;
	};


	class CAfxDllImportFuncOrdinalAndName
	{
	public:
		CAfxDllImportFuncOrdinalAndName(DWORD ordinal, LPCSTR name)
			: m_Ordinal(ordinal)
			, m_Name(name)
		{
		}

		bool operator<(const CAfxDllImportFuncOrdinalAndName& y) const {
			if( m_Ordinal < y.m_Ordinal)
				return true;

			return strcmp(m_Name, y.m_Name) < 0;
		}

	private:
		DWORD m_Ordinal;
		LPCSTR m_Name;
	};

	BOOL m_Okay = FALSE;
	LPCSTR m_Name;
	std::map<CAfxDllImportFuncName, CAfxImportFuncHookBase*> m_FuncHooksByName;
	std::map<CAfxDllImportFuncOrdinal, CAfxImportFuncHookBase*> m_FuncHooksByOrdinal;
	std::map<CAfxDllImportFuncOrdinalAndName, CAfxImportFuncHookBase*> m_FuncHooksByOrdinalAndName;

};

typedef std::vector<CAfxImportDllHook*> CAfxImportsHooks;

class CAfxImportsHook
{
public:
	CAfxImportsHook()
	{

	}

	CAfxImportsHook(const CAfxImportsHooks& hooks)
	{
		Add(hooks);
	}

	void Add(const CAfxImportsHooks& hooks)
	{
		for (std::vector<CAfxImportDllHook*>::const_iterator it = hooks.begin(); it != hooks.end(); ++it)
		{
			CAfxImportDllHook* hook = *it;

			m_Imports[hook->GetName()] = hook;
		}
	}

	BOOL Apply(HMODULE hModule);

	BOOL Unapply(HMODULE hModule);

private:
	std::map<std::string, CAfxImportDllHook*> m_Imports;

	CAfxImportDllHook* m_CurrentImportDllHook;

	static BOOL CALLBACK DetourImportFileCallback(PVOID pContext, HMODULE hModule, LPCSTR pszFile);
	static BOOL CALLBACK ApplyDetourImportFuncCallbackEx(PVOID pContext, DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc);
	static BOOL CALLBACK UnapplyDetourImportFuncCallbackEx(PVOID pContext, DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc);
};

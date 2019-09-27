#include "stdafx.h"

#include <shared/AfxDetours.h>

//TODO: MdtAllocExecuteableMemory needs probably FlushInstructionCache right after when used
// but we currently only obey that in DetourClassFunc.

//#define MDT_DEBUG

#define JMP32_SZ	5	// the size of JMP <address>
#define POPREG_SZ	1	// the size of a POP <reg>
#define NOP			0x90 // opcode for NOP
#define JMP			0xE9 // opcode for JUMP
#define POP_EAX		0x58
#define POP_ECX		0x59
#define PUSH_EAX	0x50
#define PUSH_ECX	0x51

LPVOID MdtAllocExecuteableMemory(size_t size)
{
	DWORD dwDummy;
	LPVOID mem = malloc(size);

	VirtualProtect(mem, size, PAGE_EXECUTE_READWRITE, &dwDummy);

	return mem;
}


void MdtMemAccessBegin(LPVOID lpAddress, size_t size, MdtMemBlockInfos *mdtMemBlockInfos)
{
	MEMORY_BASIC_INFORMATION mbi;
	MdtMemBlockInfo mbi2;
	DWORD dwDummy;

	LPVOID lpEnd = (BYTE *)lpAddress + size;

	while(lpAddress < lpEnd)
	{
		if(VirtualQuery(lpAddress, &mbi, sizeof(mbi)))
		{
			mbi2.BaseAddress = mbi.BaseAddress;
			mbi2.Protect = mbi.Protect;
			mbi2.RegionSize = mbi.RegionSize;

			mdtMemBlockInfos->push_back(mbi2);
		} else
			break;

		lpAddress = (BYTE *)mbi.BaseAddress + mbi.RegionSize;
	}

	for(size_t i=0; i < mdtMemBlockInfos->size(); i++) {
		mbi2 = mdtMemBlockInfos->at(i);
		VirtualProtect(mbi2.BaseAddress, mbi2.RegionSize, PAGE_EXECUTE_READWRITE, &dwDummy);
	}
}

void MdtMemAccessEnd(MdtMemBlockInfos *mdtMemBlockInfos)
{
	MdtMemBlockInfo mbi2;
	DWORD dwDummy;

	HANDLE currentProcess = GetCurrentProcess();

	for(size_t i=0; i < mdtMemBlockInfos->size(); i++) {
		mbi2 = mdtMemBlockInfos->at(i);
		FlushInstructionCache(currentProcess, mbi2.BaseAddress, mbi2.RegionSize);
		VirtualProtect(mbi2.BaseAddress, mbi2.RegionSize, mbi2.Protect, &dwDummy);
	}

}

// Detour
void *DetourApply(BYTE *orig, BYTE *hook, int len)
{
	MdtMemBlockInfos mbis;
	BYTE *jmp = (BYTE*)MdtAllocExecuteableMemory(len+JMP32_SZ);

	MdtMemAccessBegin(orig, len, &mbis);

	memcpy(jmp, orig, len);

	jmp += len; // increment to the end of the copied bytes
	jmp[0] = JMP;
	*(DWORD*)(jmp+1) = (DWORD)(orig+len - jmp) - JMP32_SZ;

	memset(orig, NOP, len);
	orig[0] = JMP;
	*(DWORD*)(orig+1) = (DWORD)(hook - orig) - JMP32_SZ;

	MdtMemAccessEnd(&mbis);

	return (jmp-len);
}

void *DetourClassFunc(BYTE *src, const BYTE *dst, const int len)
{
	BYTE *jmp = (BYTE*)MdtAllocExecuteableMemory(len+JMP32_SZ+POPREG_SZ+POPREG_SZ+POPREG_SZ);
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin(src, len, &mbis);

	memcpy(jmp+3, src, len);

	// calculate callback function call
	jmp[0] = POP_EAX;						// pop eax
	jmp[1] = POP_ECX;						// pop ecx
	jmp[2] = PUSH_EAX;						// push eax
	jmp[len+3] = JMP;						// jmp
	*(DWORD*)(jmp+len+4) = (DWORD)((src+len) - (jmp+len+3)) - JMP32_SZ;

	// detour source function call
	src[0] = POP_EAX;						// pop eax;
	src[1] = PUSH_ECX;						// push ecx
	src[2] = PUSH_EAX;						// push eax
	src[3] = JMP;							// jmp
	*(DWORD*)(src+4) = (DWORD)(dst - (src+3)) - JMP32_SZ;

	memset(src+8, NOP, len - 8);

	MdtMemAccessEnd(&mbis);

	return jmp;
}


void *DetourVoidClassFunc(BYTE *src, const BYTE *dst, const int len)
{
	BYTE *jmp = (BYTE*)MdtAllocExecuteableMemory(len+JMP32_SZ+POPREG_SZ);
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin(src, len, &mbis);

	memcpy(jmp+1, src, len);

	// calculate callback function call
	jmp[0] = POP_ECX;						// pop ecx
	jmp[len+1] = JMP;						// jmp
	*(DWORD*)(jmp+len+2) = (DWORD)((src+len) - (jmp+len+1)) - JMP32_SZ;

	// detour source function call
	src[0] = 0x87; // XCHG ecx, [esp]
	src[1] = 0x0c; // .
	src[2] = 0x24; // .
	src[3] = PUSH_ECX; // push ecx
	src[4] = JMP;							// jmp
	*(DWORD*)(src+5) = (DWORD)(dst - (src+4)) - JMP32_SZ;

	memset(src+9, NOP, len - 9);

	MdtMemAccessEnd(&mbis);

	return jmp;
}


void DetourIfacePtr(DWORD * ptr, void const * hook, DetourIfacePtr_fn & outTarget)
{
	MdtMemBlockInfos mbis;
	DWORD orgAddr;
	HANDLE hCurrentProcss = GetCurrentProcess();

	MdtMemAccessBegin(ptr, sizeof(DWORD), &mbis);

	orgAddr = *ptr;

	BYTE *jmpTarget = (BYTE*)MdtAllocExecuteableMemory(JMP32_SZ+POPREG_SZ+POPREG_SZ+POPREG_SZ);

	// padding code that jumps to target:
	jmpTarget[0] = POP_EAX;						// pop eax
	jmpTarget[1] = POP_ECX;						// pop ecx
	jmpTarget[2] = PUSH_EAX;						// push eax
	jmpTarget[3] = JMP;						// jmp
	*(DWORD*)(jmpTarget+4) = (orgAddr - (DWORD)(jmpTarget+3)) - JMP32_SZ;

	FlushInstructionCache(hCurrentProcss, jmpTarget, JMP32_SZ + POPREG_SZ + POPREG_SZ + POPREG_SZ);


	outTarget = (void(*)(void))jmpTarget;


	BYTE * jmpHook = (BYTE*)MdtAllocExecuteableMemory(JMP32_SZ+POPREG_SZ+POPREG_SZ+POPREG_SZ);

	// padding code that jumps to our hook:
	jmpHook[0] = POP_EAX;						// pop eax;
	jmpHook[1] = PUSH_ECX;						// push ecx
	jmpHook[2] = PUSH_EAX;						// push eax
	jmpHook[3] = JMP;							// jmp
	*(DWORD*)(jmpHook+4) = (DWORD)((BYTE *)hook - (jmpHook+3)) - JMP32_SZ;

	FlushInstructionCache(hCurrentProcss, jmpHook, JMP32_SZ + POPREG_SZ + POPREG_SZ + POPREG_SZ);

	// update iface ptr:
	*ptr = (DWORD)jmpHook; // this needs to be an atomic operation!!! (currently is)

	MdtMemAccessEnd(&mbis);
}


void Asm32ReplaceWithJmp(void * replaceAt, size_t countBytes, void * jmpTo)
{
	MdtMemBlockInfos mbis;

	MdtMemAccessBegin(replaceAt, countBytes, &mbis);

	memset(replaceAt, NOP, countBytes);
	((BYTE *)replaceAt)[0] = JMP;
	*(DWORD*)((BYTE *)replaceAt+1) = (DWORD)((BYTE *)jmpTo - (BYTE *)replaceAt) - JMP32_SZ;

	MdtMemAccessEnd(&mbis);
}

BOOL CAfxImportFuncHookBase::Apply(PVOID* ppvFunc)
{
	DWORD oldProtect;

	if (VirtualProtect(ppvFunc, sizeof(*ppvFunc),PAGE_EXECUTE_READWRITE,&oldProtect))
	{
		*GetTrueFunc() = *ppvFunc;
		*ppvFunc = *GetMyFunc();

		return VirtualProtect(ppvFunc, sizeof(*ppvFunc), oldProtect, NULL);
	}

	return FALSE;
}

BOOL CAfxImportFuncHookBase::Unapply(PVOID* ppvFunc)
{
	DWORD oldProtect;

	if (*GetTrueFunc())
	{
		if (VirtualProtect(ppvFunc, sizeof(*ppvFunc), PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			*ppvFunc = *GetTrueFunc();
			*GetTrueFunc() = nullptr;

			return VirtualProtect(ppvFunc, sizeof(*ppvFunc), oldProtect, NULL);
		}

		return FALSE;
	}

	return TRUE;
}

BOOL CAfxImportDllHook::Apply(DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc)
{
	if (nullptr == ppvFunc) return TRUE;

	m_Okay = TRUE;
	if (pszFunc)
	{
		std::map<CAfxDllImportFuncOrdinalAndName, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByOrdinalAndName.find(CAfxDllImportFuncOrdinalAndName(nOrdinal, pszFunc));
		if(it != m_FuncHooksByOrdinalAndName.end())
		{
			if (!(it->second)->Apply(ppvFunc)) m_Okay = FALSE;
		}
	}
	{
		std::map<CAfxDllImportFuncOrdinal, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByOrdinal.find(CAfxDllImportFuncOrdinal(nOrdinal));
		if (it != m_FuncHooksByOrdinal.end())
		{
			if (!(it->second)->Apply(ppvFunc)) m_Okay = FALSE;
		}
	}
	if (pszFunc)
	{
		std::map<CAfxDllImportFuncName, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByName.find(CAfxDllImportFuncName(pszFunc));
		if (it != m_FuncHooksByName.end())
		{
			if (!(it->second)->Apply(ppvFunc)) m_Okay = FALSE;
		}
	}

	return TRUE;
}

BOOL CAfxImportDllHook::Unapply(DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc)
{
	if (nullptr == ppvFunc) return TRUE;

	m_Okay = TRUE;
	if(pszFunc)
	{
		std::map<CAfxDllImportFuncOrdinalAndName, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByOrdinalAndName.find(CAfxDllImportFuncOrdinalAndName(nOrdinal, pszFunc));
		if (it != m_FuncHooksByOrdinalAndName.end())
		{
			if (!(it->second)->Unapply(ppvFunc)) m_Okay = FALSE;
		}
	}
	{
		std::map<CAfxDllImportFuncOrdinal, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByOrdinal.find(CAfxDllImportFuncOrdinal(nOrdinal));
		if (it != m_FuncHooksByOrdinal.end())
		{
			if (!(it->second)->Unapply(ppvFunc)) m_Okay = FALSE;
		}
	}
	if (pszFunc)
	{
		std::map<CAfxDllImportFuncName, CAfxImportFuncHookBase*>::iterator it = m_FuncHooksByName.find(CAfxDllImportFuncName(pszFunc));
		if (it != m_FuncHooksByName.end())
		{
			if (!(it->second)->Unapply(ppvFunc)) m_Okay = FALSE;
		}
	}

	return TRUE;
}

BOOL CALLBACK CAfxImportsHook::DetourImportFileCallback(PVOID pContext, HMODULE hModule, LPCSTR pszFile)
{
	if (nullptr == pszFile) return TRUE;

	CAfxImportsHook* importsHook = reinterpret_cast<CAfxImportsHook*>(pContext);
	std::map<std::string, CAfxImportDllHook*>::iterator it = importsHook->m_Imports.find(pszFile);
	importsHook->m_CurrentImportDllHook = it != importsHook->m_Imports.end() ? it->second : nullptr;

	return TRUE;
}

BOOL CALLBACK CAfxImportsHook::ApplyDetourImportFuncCallbackEx(PVOID pContext, DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc)
{
	CAfxImportsHook* importsHook = reinterpret_cast<CAfxImportsHook*>(pContext);
	if (importsHook->m_CurrentImportDllHook) {
		importsHook->m_CurrentImportDllHook->Apply(nOrdinal, pszFunc, ppvFunc);
	}

	return TRUE;
}

BOOL CALLBACK CAfxImportsHook::UnapplyDetourImportFuncCallbackEx(PVOID pContext, DWORD nOrdinal, LPCSTR pszFunc, PVOID* ppvFunc)
{
	CAfxImportsHook* importsHook = reinterpret_cast<CAfxImportsHook*>(pContext);
	if (importsHook->m_CurrentImportDllHook) importsHook->m_CurrentImportDllHook->Unapply(nOrdinal, pszFunc, ppvFunc);

	return TRUE;
}

BOOL CAfxImportsHook::Apply(HMODULE hModule)
{
	return DetourEnumerateImportsEx(hModule, this, DetourImportFileCallback, ApplyDetourImportFuncCallbackEx);
}

BOOL CAfxImportsHook::Unapply(HMODULE hModule)
{
	return DetourEnumerateImportsEx(hModule, this, DetourImportFileCallback, UnapplyDetourImportFuncCallbackEx);
}

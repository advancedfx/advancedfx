#include "stdafx.h"

#include "MaterialSystemHooks.h"

#include "addresses.h"

#include <shared/AfxDetours.h>

#include <atomic>

void * g_pMaterialSystem = nullptr;
typedef void * (__fastcall * CMaterialSystem_GetRenderCallQueue_t)(void * This, void * Edx);
CMaterialSystem_GetRenderCallQueue_t g_Old_CMaterialSystem_GetRenderCallQueue = nullptr;
void * __fastcall New_CMaterialSystem_GetRenderCallQueue(void * This, void * Edx) {
    g_pMaterialSystem = This;
    return g_Old_CMaterialSystem_GetRenderCallQueue(This,Edx);
}

typedef void (__fastcall * CMatCallQueue_QueueFunctor_t)(void * This, void * Edx, void * pFunctor);

class IMaterialSystemRefCounted {
public:
	virtual int AddRef() = 0;
	virtual int Release() = 0;
};

class CMaterialSystemFunctor3 : public IMaterialSystemRefCounted {
public:
    CMaterialSystemFunctor3(CMaterialSystemFunctor * functor)
    : m_Functor(functor) {

    }

	virtual int AddRef() {
        int value = ++m_RefCount;
        return value;
    }

	virtual int Release() {
        int value = --m_RefCount;
        if(0 == value) {
            delete m_Functor;
            delete this;
        }
        return value;
    }
	
    virtual void operator()() {
        m_Functor->operator()();
    }

private:
	unsigned int m_nUserID = 0; // For debugging
    CMaterialSystemFunctor * m_Functor;
    std::atomic_int m_RefCount = 0;
};

class CMaterialSystemFunctor4 : public IMaterialSystemRefCounted {
public:
    CMaterialSystemFunctor4(CMaterialSystemFunctor * functor)
    : m_Functor(functor) {

    }

	virtual int AddRef() {
        int value = ++m_RefCount;
        return value;
    }

	virtual int Release() {
        int value = --m_RefCount;
        if(0 == value) {
            delete m_Functor;
            delete this;
        }
        return value;
    }
	
    virtual ~CMaterialSystemFunctor4() {};    

    virtual void operator()() {
        m_Functor->operator()();
    }

private:
	unsigned int m_nUserID = 0; // For debugging
    CMaterialSystemFunctor * m_Functor;
    std::atomic_int m_RefCount = 0;
};

bool MaterialSystem_ExecuteOnRenderThread(CMaterialSystemFunctor * pFunctor) {
    if(g_pMaterialSystem && AFXADDR_GET(materialsystem_GetRenderCallQueue)) {
        if(3 == AFXADDR_GET(materialsystem_CFunctor_vtable_size)) {
            if(void * pCallQueue =  g_Old_CMaterialSystem_GetRenderCallQueue(g_pMaterialSystem, 0)) {
                CMaterialSystemFunctor3 * pFunctor2 = new CMaterialSystemFunctor3(pFunctor);
                pFunctor2->AddRef();
                CMatCallQueue_QueueFunctor_t pQueueFunctorInternal = (CMatCallQueue_QueueFunctor_t)AFXADDR_GET(materialsystem_CMatCallQueue_QueueFunctor);
                pQueueFunctorInternal(pCallQueue,0,pFunctor2);
                pFunctor2->Release();
            } else {
                pFunctor->operator()();
                delete pFunctor;
            }
            return true;
        } else if(4 == AFXADDR_GET(materialsystem_CFunctor_vtable_size)) {
            if(void * pCallQueue =  g_Old_CMaterialSystem_GetRenderCallQueue(g_pMaterialSystem, 0)) {
                CMaterialSystemFunctor4 * pFunctor2 = new CMaterialSystemFunctor4(pFunctor);
                pFunctor2->AddRef();
                switch(g_SourceSdkVer) {
                case SourceSdkVer_L4D2:
                case SourceSdkVer_TF2:
                case SourceSdkVer_HL2MP:
                    pFunctor2->AddRef();
                    break;
                }
                CMatCallQueue_QueueFunctor_t pQueueFunctorInternal = (CMatCallQueue_QueueFunctor_t)AFXADDR_GET(materialsystem_CMatCallQueue_QueueFunctor);
                size_t this_offset = 4;
                switch(g_SourceSdkVer){
                    case SourceSdkVer_TF2:
                    case SourceSdkVer_HL2MP:
                        this_offset = 0xa4;
                        break;
                    default:
                        this_offset = 0x0;
                        break;
                }
                pQueueFunctorInternal((unsigned char*)pCallQueue+this_offset,0,pFunctor2);
                pFunctor2->Release();
            } else {
                pFunctor->operator()();
                delete pFunctor;
            }
            return true;
        }
    }
    else if(g_SourceSdkVer == SourceSdkVer_CSSV34) {
        // This game is non-queued (luckily)
        pFunctor->operator()();
        delete pFunctor;
        return true;
    }
    delete pFunctor;
    return false;
}

bool Hook_MaterialSystem_GetRenderCallQueue(void)
{
	static bool firstRun = true;
	static bool firstResult = false;

	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(materialsystem_GetRenderCallQueue)
        && AFXADDR_GET(materialsystem_CMatCallQueue_QueueFunctor)
        && (
            3 == AFXADDR_GET(materialsystem_CFunctor_vtable_size)
            || 4 == AFXADDR_GET(materialsystem_CFunctor_vtable_size)
        )
    ) {
        g_Old_CMaterialSystem_GetRenderCallQueue = (CMaterialSystem_GetRenderCallQueue_t)AFXADDR_GET(materialsystem_GetRenderCallQueue);
    
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Old_CMaterialSystem_GetRenderCallQueue, New_CMaterialSystem_GetRenderCallQueue);

        firstResult = NO_ERROR == DetourTransactionCommit();

        firstResult = true;
	}

	return firstResult;
}

void OnBefore_CMaterialSystem_SwappBuffers(void);

typedef void (__fastcall * CMaterialSystem_SwapBuffers_t)(void * This, void * Edx);
CMaterialSystem_SwapBuffers_t g_Old_CMaterialSystem_SwapBuffers = nullptr;
void __fastcall New_CMaterialSystem_SwapBuffers(void * This, void * Edx) {
    g_pMaterialSystem = This;
    OnBefore_CMaterialSystem_SwappBuffers();
    return g_Old_CMaterialSystem_SwapBuffers(This,Edx);
}

bool Hook_MaterialSystem_SwapBuffers(void)
{
   	static bool firstRun = true;
	static bool firstResult = false;

	if(!firstRun) return firstResult;
	firstRun = false;

	if(AFXADDR_GET(materialsystem_CMaterialSystem_SwapBuffers)) {
        g_Old_CMaterialSystem_SwapBuffers = (CMaterialSystem_SwapBuffers_t)AFXADDR_GET(materialsystem_CMaterialSystem_SwapBuffers);
    
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_Old_CMaterialSystem_SwapBuffers, New_CMaterialSystem_SwapBuffers);

        firstResult = NO_ERROR == DetourTransactionCommit();
	}

	return firstResult; 
}


bool Hook_MaterialSystem(void)
{
    bool bOk = true;
    if(g_SourceSdkVer != SourceSdkVer_CSSV34
        && !Hook_MaterialSystem_GetRenderCallQueue()
    ) bOk = false;
    if(!Hook_MaterialSystem_SwapBuffers()) bOk = false;
    return bOk;
}

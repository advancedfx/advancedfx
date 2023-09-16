#pragma once

class CMaterialSystemFunctor abstract {
public:
    virtual ~CMaterialSystemFunctor() {};
    virtual void operator()() = 0;
};

bool MaterialSystem_ExecuteOnRenderThread(CMaterialSystemFunctor * functor);

bool Hook_MaterialSystem_GetRenderCallQueue(void);
bool Hook_MaterialSystem_SwapBuffers(void);
bool Hook_MaterialSystem(void);

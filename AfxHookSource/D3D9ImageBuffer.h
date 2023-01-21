#pragma once

#include <d3d9.h>
#include <shared/AfxImageBuffer.h>


class IAfxD3D9OnRelease abstract {
public:
    virtual void AfxD3D9OnRelease() = 0;
};

/**
 * Must be called from GPU thread only.
 */
void AfxD3D9OnReleaseAdd(IAfxD3D9OnRelease * value);

/**
 * Must be called from GPU thread only.
 */
void AfxD3D9OnReleaseRemove(IAfxD3D9OnRelease * value);

/**
 * Thread safe. Block / defers triggering of OnRelease.
 */
void AfxD3D9OnReleaseLock();

/**
 * Therad safe.
 */
void AfxD3D9OnReleaseUnock();

/**
 * Must be released upon AfxD3D9OnRelease.
 * Must be only called on GPU thread.
 */
class IAfxD3D9Capture abstract {
public:
    virtual void AddRef() = 0;

    virtual void Release() = 0;

    /**
     * Capture.
     */
    virtual bool Capture() = 0;

    /**
     * Start accessing system memory surface.
     */
    virtual const advancedfx::IImageBuffer * LockCpu() = 0;

    /**
     * End accessing system memory surface.
     */
    virtual void UnlockCpu() = 0;    
};

IAfxD3D9Capture * AfxD3d9CreateRenderTargetCompatibleCapture();
#pragma once

#include <d3d9.h>
#include <shared/ImageBufferThreadSafe.h>


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


class IAfxD3D9CaptureBuffer abstract {
public:
    /**
     * @remarks thread-safe
    */
    virtual void AddRef() = 0;

    /**
     * @remarks thread-safe
    */
    virtual void Release() = 0;

    /**
     * Start accessing system memory surface.
     * @returns nullptr on failure, otherwise ImageBuffer that needs to be released when you are done.
     */
    virtual advancedfx::IImageBufferThreadSafe * LockCpu() = 0;
};

/**
 * Must be released upon AfxD3D9OnRelease..
 */
class IAfxD3D9Capture abstract {
public:
    /**
     * Capture.
     * @remarks Must be only called on GPU thread.
     */
    virtual void AddRef() = 0;

    /**
     * Capture.
     * @remarks Must be only called on GPU thread.
     */
    virtual void Release() = 0;

    /**
     * Capture.
     * @remarks Must be only called on GPU thread.
     */
    virtual IAfxD3D9CaptureBuffer * Capture() = 0;
};

IAfxD3D9Capture * AfxD3d9CreateRenderTargetCompatibleCapture();

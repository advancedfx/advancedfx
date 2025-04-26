#pragma once

#include "Captures.h"
#include "ThreadPool.h"

namespace advancedfx {
namespace ImageTransformer {

	class ICapture* StripAlpha(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture);

	class ICapture* RgbaToBgr(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture);

	class ICapture* RgbaToBgra(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture);

	class ICapture* DepthF(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture, float depthScale, float depthOfs);

	class ICapture* Depth24(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture, float depthScale, float depthOfs);

	class ICapture* Matte(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* captureEntBlack, class ICapture* captureEntWhite);

	class ICapture* AColorBRedAsAlpha(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* aColor, class ICapture* bRedAsAlpha);

} // namespace ImageTransformer {
} // namespace advancedfx

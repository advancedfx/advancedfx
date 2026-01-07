#pragma once

#include "ImageBufferThreadSafe.h"
#include "GrowingBufferPoolThreadSafe.h"
#include "ThreadPool.h"

namespace advancedfx {
namespace ImageTransformer {

	IImageBufferThreadSafe* StripAlpha(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer);

	IImageBufferThreadSafe* RgbaToBgr(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer);

	IImageBufferThreadSafe* RgbaToBgra(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer);

	IImageBufferThreadSafe* DepthF(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer, float depthScale, float depthOfs);

	IImageBufferThreadSafe* Depth24(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer, float depthScale, float depthOfs);

	IImageBufferThreadSafe* Matte(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* bufferEntBlack, IImageBufferThreadSafe* bufferEntWhite);

	IImageBufferThreadSafe* AColorBRedAsAlpha(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* aColor, IImageBufferThreadSafe* bRedAsAlpha);

} // namespace ImageTransformer {
} // namespace advancedfx

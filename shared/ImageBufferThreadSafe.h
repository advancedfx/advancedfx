#pragma once

#include "ImageFormat.h"
#include "TImageBuffer.h"

namespace advancedfx {
    typedef TIImageBuffer<true> IImageBufferThreadSafe;
    typedef TImageBuffer<true> CImageBufferThreadSafe;
}

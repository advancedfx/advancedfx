#pragma once

// Copyright (c) by advancedfx.org
//
// Last changes:
// 2014-01-30 dominik.matrixstorm.com
//
// First changes
// 2014-01-30 dominik.matrixstorm.com

#include "AfxGlImage.h"

class AfxImageUtils
{
public:
	static bool DebugFloatDepthBuffer(AfxGlImage * image);

	static bool FloatDepthBufferToByteBuffer(AfxGlImage * image);

	static bool InverseFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar);

	static bool LinearizeFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar);

	static bool LogarithmizeFloatDepthBuffer(AfxGlImage * image, GLdouble zNear, GLdouble zFar);

	static bool SliceFloatDepthBuffer(AfxGlImage * image, GLdouble sliceLo, GLdouble sliceHi);

	static bool WriteBitmap(AfxGlImage * image, wchar_t const * fileName);

private:
	static bool IsEasyFloatDepthBuffer(AfxGlImage * image);
};

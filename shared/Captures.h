#pragma once

#include "AfxImageBuffer.h"

namespace advancedfx {

class ICapture {
public:
	/**
	 * Thread safe.
	 */
	virtual void AddRef() = 0;

	/**
	 * Thread safe.
	 */
	virtual void Release() = 0;

	/**
	 * Thread safe.
	 * @remark reference count remains unchanged.
	 * @return nullptr on failure.
	 */
	virtual const IImageBuffer* GetBuffer() const = 0;
};

} // namespace advancedfx

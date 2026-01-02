#ifndef MDT_MEDIA_H
#define MDT_MEDIA_H

#include "../shared/GrowingBufferPool.h"
#include "../shared/ImageBuffer.h"
#include "../shared/ImageFormat.h"
#include "../shared/RefCounted.h"

#include <windows.h>
#include <gl\gl.h>

class CMdt_Media_RAWGLPIC
{
public:
	enum class Error : unsigned int {
		None = 0,
		Memory = 2,
		ImageFormat = 7
	};

	/**
	 * glReadPixels - we always use glCoords here
	 * @remarks Requires pitch is a multiple of 4 and ImageOrigin::TopLeft.
	 */
	static Error DoGlReadPixels(int iXofs, int iYofs, const advancedfx::CImageFormat &imageFormat, advancedfx::CImageBuffer * pImageBuffer);
	
//	void			RepeatGlReadPixels();				  // equals DoGlReadPixels(0,0)
//	void			RepeatGlReadPixels(int iXofs, int iYofs); // performfs glReadPixels with an optional offset and the params you have set when this class was created
};


#endif
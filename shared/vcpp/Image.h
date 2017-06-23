#pragma once

#include <gl/gl.h>

#include "Data.h"
#include "Ref.h"

namespace Afx {

struct __declspec(novtable) ICapture abstract
{
	virtual void Capture() abstract = 0;
	virtual IRef * Ref() abstract = 0;
};

/// <summary> Image with 8bit components </summary>
/// <remarks> Guarantees memory layout only, other semantics are obligatory. </remarks>
struct __declspec(novtable) IImageX8 abstract
{
	virtual IData * Data() abstract = 0;
	virtual int Height() abstract = 0;
	virtual int Pitch() abstract = 0;
	virtual IRef * Ref() abstract = 0;
	virtual int Width() abstract = 0;
};

/// <summary> Image with 16bit components </summary>
/// <remarks> Guarantees memory layout only, other semantics are obligatory. </remarks>
struct __declspec(novtable) IImageX16 abstract
{
	virtual IData * Data() abstract = 0;
	virtual int Height() abstract = 0;
	virtual int Pitch() abstract = 0;
	virtual IRef * Ref() abstract = 0;
	virtual int Width() abstract = 0;
};

/// <summary> Image with 24bit components </summary>
/// <remarks> Guarantees memory layout only, other semantics are obligatory. </remarks>
struct __declspec(novtable) IImageX24 abstract
{
	virtual IData * Data() abstract = 0;
	virtual int Height() abstract = 0;
	virtual int Pitch() abstract = 0;
	virtual IRef * Ref() abstract = 0;
	virtual int Width() abstract = 0;
};

/// <summary> Image with 32bit components </summary>
/// <remarks> Guarantees memory layout only, other semantics are obligatory. </remarks>
struct __declspec(novtable) IImageX32 abstract
{
	virtual IData * Data() abstract = 0;
	virtual int Height() abstract = 0;
	virtual int Pitch() abstract = 0;
	virtual IRef * Ref() abstract = 0;
	virtual int Width() abstract = 0;
};

/// <summary> Image with IEEE-Single components </summary>
/// <remarks> Guarantees memory layout only, other semantics are obligatory. </remarks>
struct __declspec(novtable) IImageXS abstract
{
	virtual IData * Data() abstract = 0;
	virtual int Height() abstract = 0;
	virtual int Pitch() abstract = 0;
	virtual IRef * Ref() abstract = 0;
	virtual int Width() abstract = 0;
};


// ImageWrapper ////////////////////////////////////////////////////////////////

class ImageWrapper
{
public:

	/// <returns> wrapped image or 0 if wrapping not possible </returns>
	static IImageX16 * ToX16(IImageX8 * image);

	/// <returns> wrapped image or 0 if wrapping not possible </returns>
	static IImageX24 * ToX24(IImageX8 * image);

	/// <returns> wrapped image or 0 if wrapping not possible </returns>
	static IImageX32 * ToX32(IImageX8 * image);

	/// <returns> wrapped image or 0 if wrapping not possible </returns>
	static IImageXS * ToXS(IImageX8 * image);
};


} // namespace Afx {

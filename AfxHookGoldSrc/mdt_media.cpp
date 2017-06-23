#include "stdafx.h"

#include "mdt_media.h"

#include <algorithm>

//#define MDT_DEBUG
#ifdef MDT_DEBUG
	#include <stdio.h> // _snprintf
	#include <malloc.h> // _msize
#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation of class CMdt_Media_BASE

// _CMdt_Media_BASE:
void CMdt_Media_BASE::_CMdt_Media_BASE()
{
	pOnErrorHandler = NULL; // by default the class user didn't set an own error handler
	_has_unhandled_error = false;
}

// CMdt_Media_BASE:
CMdt_Media_BASE::CMdt_Media_BASE()
{
	_CMdt_Media_BASE(); // call stuff all constructors should do

	// store media diretion and type:
	_media_direction = MD_UNKOWN;
	_media_type      = MT_UNKOWN;
}

// CMdt_Media_BASE:
CMdt_Media_BASE::CMdt_Media_BASE(MEDIA_TYPE eMediaType, MEDIA_DIRECTION eMediaDirection)
{
	_CMdt_Media_BASE(); // call stuff all constructors should do

	// store media type and direction
	_media_type      = eMediaType;
	_media_direction = eMediaDirection;
}

// GetMediaDirection:
CMdt_Media_BASE::MEDIA_DIRECTION CMdt_Media_BASE::GetMediaDirection()
{
	return _media_direction;
}

// GetMediaType:
CMdt_Media_BASE::MEDIA_TYPE CMdt_Media_BASE::GetMediaType()
{
	return _media_type;
}

// SetMediaDirection:
void CMdt_Media_BASE::SetMediaDirection (MEDIA_DIRECTION eMediaDirection)
{
	_media_direction = eMediaDirection;
}

// SetMediaType:
void CMdt_Media_BASE::SetMediaType(MEDIA_TYPE eMediaType)
{
	_media_type = eMediaType;
}

// _OnErrorHandler:
MDT_MEDIA_ERROR_ACTION CMdt_Media_BASE::_OnErrorHandler(unsigned int iErrorCode)
{
	if (pOnErrorHandler) return pOnErrorHandler(iErrorCode); // call the error handler if present
	else
	{
		// there is no error handler present, so we handle it internally:
		_last_unhandled_error = iErrorCode;
		_has_unhandled_error  = true;
		return MMOEA_DEFAULT; // let class decide the action to take
	}
}

//
unsigned int CMdt_Media_BASE::GetLastUnhandledError()
{
	if (_has_unhandled_error) return _last_unhandled_error;
	return MDT_MEDIA_ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of class CMdt_Media_RAWGLPIC

// _GetSizeComponent:
bool CMdt_Media_RAWGLPIC::_CalcSizeComponent (GLenum eGLtype, unsigned char* outSizeComponent, bool* outIsSigned)
{
	// well this is not too nice, replace it if u want (may be there is a native GL or GLU header file that already has functions for that:
	// see GL reference for glDrawPixels
	switch(eGLtype)
	{
	case GL_UNSIGNED_BYTE:
		*outIsSigned      = false;
		*outSizeComponent = 1;
		return true;
	case GL_BYTE:
		*outIsSigned      = true;
		*outSizeComponent = 1;
		return true;
//	case GL_BITMAP:
		// I am to lazy to code that now, this would require special treatment determining the bitdepth etc.
//		return false; // not treated yet
	case GL_UNSIGNED_SHORT:
		*outIsSigned      = false;
		*outSizeComponent = 2;
		return true;
	case GL_SHORT:
		*outIsSigned      = true;
		*outSizeComponent = 2;
		return true;
	case GL_UNSIGNED_INT:
		*outIsSigned      = false;
		*outSizeComponent = 4;
		return true;
	case GL_INT:
		*outIsSigned      = true;
		*outSizeComponent = 4;
		return true;
	case GL_FLOAT:
		*outIsSigned      = true;
		*outSizeComponent = 4;
		return true;
	default:
		return false;
	}
}

// _GetNumComponents:
bool CMdt_Media_RAWGLPIC::_CalcNumComponents (GLenum eGLformat, unsigned char* outNumComponents)
{
	// well this is not too nice, replace it if u want:
	// see http://www.glprogramming.com/blue/ch05.html#id5527285 (OGL ref. for glReadPixels)
	switch(eGLformat)
	{
	case GL_COLOR_INDEX:
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		*outNumComponents = 1;
		return true;
	case GL_BGR_EXT:
	case GL_RGB:
		*outNumComponents = 3;
		return true;
	case GL_BGRA_EXT:
	case GL_RGBA:
		*outNumComponents = 4;
		return true;
	case GL_LUMINANCE_ALPHA:
		*outNumComponents = 2; // WARNING POSSIBLE ERROR - I hope I understood that right heh
		return true;
	default:
		return false;
	}
}

// CMdt_MediaDesc_RAWGLPIC:
CMdt_Media_RAWGLPIC::CMdt_Media_RAWGLPIC()
                    :CMdt_Media_BASE(MT_RAWGLPIC,MD_IN_AND_OUT)
{
	_pBuffer    = NULL;	// this will make _freeAndClean justClean : )
	_freeAndClean();
}

// ~CMdt_MediaDesc_RAWGLPIC:
CMdt_Media_RAWGLPIC::~CMdt_Media_RAWGLPIC()
{
	_freeAndClean();
}

void CMdt_Media_RAWGLPIC::swap(CMdt_Media_RAWGLPIC& o)
{
	#define SWAP(x) std::swap(x, o.x)

	SWAP(_pBuffer);
	SWAP(_uiSize);
	SWAP(_uiBytesAllocated);

	SWAP(_bHasConsistentData);
	SWAP(_iWidth);
	SWAP(_iHeight);

	SWAP(_eGLformat);
	SWAP(_eGLtype);

	SWAP(_uiPixelSize);
	SWAP(_ucSizeComponent);
	SWAP(_ucNumComponents);
	SWAP(_bComponentIsSigned);

	#undef SWAP
}

CMdt_Media_RAWGLPIC::CMdt_Media_RAWGLPIC(const CMdt_Media_RAWGLPIC& o)
	: CMdt_Media_RAWGLPIC()
{
	_adjustMemory(o._uiSize, false);
	std::memcpy(_pBuffer, o._pBuffer, _uiSize);

	_bHasConsistentData = o._bHasConsistentData;
	_iWidth = o._iWidth;
	_iHeight = o._iHeight;

	_eGLformat = o._eGLformat;
	_eGLtype = o._eGLtype;

	_uiPixelSize = o._uiPixelSize;
	_ucSizeComponent = o._ucSizeComponent;
	_ucNumComponents = o._ucNumComponents;
	_bComponentIsSigned = o._bComponentIsSigned;
}

CMdt_Media_RAWGLPIC::CMdt_Media_RAWGLPIC(CMdt_Media_RAWGLPIC&& o)
	: CMdt_Media_RAWGLPIC()
{
	swap(o);
}

CMdt_Media_RAWGLPIC& CMdt_Media_RAWGLPIC::operator=(CMdt_Media_RAWGLPIC rhs)
{
	swap(rhs);
	return *this;
}

// DoGlReadPixels:
bool CMdt_Media_RAWGLPIC::DoGlReadPixels(int iXofs, int iYofs, int iWidth, int iHeight, GLenum eGLformat, GLenum eGLtype, bool bRepack)
{
#ifdef MDT_DEBUG
	static char sztmp[2000];
#endif

	// calcualte data required to know the size of a pixel (if possible / known):
	if (_eGLformat!=eGLformat)
		if (!_CalcNumComponents(eGLformat,&_ucNumComponents))
		{
			_bHasConsistentData	= false; // data not consistent anynmore, but no need to free everything yet
			_OnErrorHandler(MDT_MEDIA_ERROR_GLFORMAT);
			return false;
		} else _eGLformat = eGLformat;
	;

	if (_eGLtype!=eGLtype)
		if (!_CalcSizeComponent(eGLtype,&_ucSizeComponent,&_bComponentIsSigned))
		{
			_bHasConsistentData	= false; // data not consistent anynmore, but no need to free everything yet
			_OnErrorHandler(MDT_MEDIA_ERROR_GLTYPE);
			return false;
		} else _eGLtype = eGLtype;
	;

	// we need to recaclualte the memory usuage and adjust the memory
	_iWidth				= iWidth;
	_iHeight			= iHeight;

	// calcualte the size:
	_uiPixelSize = ((unsigned short)_ucNumComponents) * ((unsigned short)_ucSizeComponent);
	_uiSize      = _iWidth * ((unsigned int)_uiPixelSize);

	// check for problems with GL_PIXEL_ALIGNMENT == 4
	unsigned int uiRowSize = _uiSize;

	if (_uiSize & 0x03)
		// is not divideable by 4 (has remainder)
		_uiSize = (1+ (_uiSize >> 2))<<2; // fill up to 4

	unsigned int uiRowPackSize = _uiSize;

	_uiSize = _iHeight * _uiSize;

	if (!_adjustMemory(_uiSize,true))  // we only want enlarging the memory if required and no compacting to keep memory access low
		return false;

#ifdef MDT_DEBUG
	_snprintf(sztmp,1000,"_uiSize: %u Bytes\niXofs: %i\niYofs: %i\n_iWidth: %i\n_iHeight: %i\n_eGLformat: 0x%08x\n_eGLtype: 0x%08x\n_pBuffer:  0x%08x (%u Bytes)",_uiSize,iXofs,iYofs,_iWidth,_iHeight,_eGLformat,_eGLtype,_pBuffer,_msize(_pBuffer));
	MessageBox(0,sztmp,"DEBUG",MB_OK);
	memset(_pBuffer,0x0FF,_uiSize);
#endif


	if (_pBuffer) glReadPixels(iXofs, iYofs, _iWidth, _iHeight, _eGLformat, _eGLtype, _pBuffer);

#ifdef MDT_DEBUG
	unsigned char* tBuffer=_pBuffer+_uiSize-1;
	unsigned int tcnt=0;
	GLint t_rowl,t_skipp,t_skiprows,t_align;

	glGetIntegerv(GL_PACK_ROW_LENGTH,&t_rowl);
	glGetIntegerv(GL_PACK_SKIP_PIXELS,&t_skipp);
	glGetIntegerv(GL_PACK_SKIP_ROWS,&t_skiprows);
	glGetIntegerv(GL_PACK_ALIGNMENT,&t_align);

	while (_pBuffer<=tBuffer && *tBuffer == 0x0FF)
	{
		tBuffer--;
		tcnt++;
	}
	_snprintf(sztmp,2000,"GL_PACK_ROW_LENGTH:%i\nGL_PACK_SKIP_PIXELS: %i\nGL_PACK_SKIP_ROWS: %i\nGL_PACK_ALIGNMENT: %i\n\niXofs: %i\niYofs: %i\n_iWidth: %i\n_iHeight: %i\n_eGLformat: 0x%08x\n_eGLtype: 0x%08x\n_pBuffer:  0x%08x (%u Bytes)\n\nExpected: %u Bytes\nAllocated: %u Bytes\nglReadPixels used: %u Bytes (minimum, might be more)",t_rowl,t_skipp,t_skiprows,t_align,iXofs,iYofs,_iWidth,_iHeight,_eGLformat,_eGLtype,_pBuffer,_msize(_pBuffer),3*_iWidth*_iHeight,_uiSize,_uiSize-tcnt);
	MessageBox(0,sztmp,"glReadPixels DEBUG",MB_OK);
#endif

	if (bRepack && (uiRowSize != uiRowPackSize))
	{
		// postprocess (it would be better to post process it when outputting, since  this way we have overhead)
		unsigned char* pTsrc=_pBuffer+uiRowPackSize;
		unsigned char* pTdst=_pBuffer+uiRowSize;
		int iT=1;

		while (iT<_iHeight)
		{
			unsigned int uR = 0;
			while(uR < uiRowSize)
			{
				pTdst[uR] = pTsrc[uR];
				uR++;
			}
			pTsrc+=uiRowPackSize;
			pTdst+=uiRowSize;
			iT++;
		}
	}


	_bHasConsistentData=true;
	return true;
}


// HasConsistentData:
bool CMdt_Media_RAWGLPIC::HasConsistentData()
{
	return _bHasConsistentData;
}

// CompactStructures:
bool CMdt_Media_RAWGLPIC::CompactStructures()
{
	return _compactMemory();
}

// FreeStructures:
void CMdt_Media_RAWGLPIC::FreeStructures()
{
	_freeAndClean();
}

// GetWidth:
int CMdt_Media_RAWGLPIC::GetWidth()
{
	return _iWidth;
}

// GetHeight:
int CMdt_Media_RAWGLPIC::GetHeight()
{
	return _iHeight;
}

// GetSize:
unsigned int CMdt_Media_RAWGLPIC::GetSize()
{
	return _uiSize;
}

// GetPointer:
unsigned char* CMdt_Media_RAWGLPIC::GetPointer()
{
	return _pBuffer;
}

// GetPixelSize:
unsigned short CMdt_Media_RAWGLPIC::GetPixelSize()
{
	return _uiPixelSize;
}

// GetNumComponents:
unsigned char CMdt_Media_RAWGLPIC::GetNumComponents()
{
	return _ucNumComponents;
}

// GetSizeComponent:
unsigned char CMdt_Media_RAWGLPIC::GetSizeComponent()
{
	return _ucSizeComponent;
}

// GetGLFormat:
GLenum CMdt_Media_RAWGLPIC::GetGLFormat()
{
	return _eGLformat;
}

// GetGLType:
GLenum CMdt_Media_RAWGLPIC::GetGLType()
{
	return _eGLtype;
}

// _adjustMemory:
bool CMdt_Media_RAWGLPIC::_adjustMemory(unsigned int iNewSize,bool bOnlyWhenGreater)
{
	if ((iNewSize>_uiBytesAllocated)||(!bOnlyWhenGreater && iNewSize != _uiBytesAllocated))
	{
		_pBuffer = (unsigned char *)realloc(_pBuffer,iNewSize);
		if(!_pBuffer && iNewSize)
		{
			_freeAndClean();
			_OnErrorHandler(MDT_MEDIA_ERROR_MEMORY);
			return false;
		} else {
			_uiBytesAllocated = iNewSize;
			_uiSize           = iNewSize;
			return true;
		}
	} else {
		_uiSize = iNewSize;
		return true;
	}
}

// _CompactMemory:
bool CMdt_Media_RAWGLPIC::_compactMemory()
{
	return _adjustMemory(_uiSize,false);
}

// _freeMemory:
void CMdt_Media_RAWGLPIC::_freeAndClean()
{
	free(_pBuffer);
	_pBuffer			= NULL;
	_bHasConsistentData	= false;
	_uiBytesAllocated	= 0;
	_uiSize				= 0;
	_iWidth				= 0;
	_iHeight			= 0;

	_eGLformat			= 0;
	_eGLtype			= 0;

	_uiPixelSize		= 0;
	_ucSizeComponent	= 0;
	_ucNumComponents	= 0;
	_bComponentIsSigned	= false;
}
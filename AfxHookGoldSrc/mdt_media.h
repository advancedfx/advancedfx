#ifndef MDT_MEDIA_H
#define MDT_MEDIA_H

#include <windows.h>
#include <gl\gl.h>

////////////////////////////////////////////////////////////////////////////////
// some defs:

// error codes:
#define MDT_MEDIA_ERROR_NONE			0 // no error happend
#define	MDT_MEDIA_ERROR_UNKOWN			1 // an unknown error happend
#define MDT_MEDIA_ERROR_MEMORY			2 // memory allocation or reallocation with i.e. malloc or new failed
//
#define MDT_MEDIA_ERROR_UNSUPPORTED		4 // the operation you requested is not supported (with the options you gave)
#define MDT_MEDIA_ERROR_GLFORMAT		5 // the GL format you choosed is not supported in the current mode
#define MDT_MEDIA_ERROR_GLTYPE			6 // the GL type you choosed is not supported in the current mode

////////////////////////////////////////////////////////////////////////////////
// Stuff that has to do with on the fly runtime error handling:
enum MDT_MEDIA_ERROR_ACTION // the classes will probably currently ignore this, it's there for upward compatibility of the code
{
	MMOEA_DEFAULT,			// let class decide the best action
	MMOEA_CONTINUE,			// try to continue with best effort (the class will try to solve or at least ignore the error with existing code if possible)
	MMOEA_EXIT				// try to break the current operations
};

typedef MDT_MEDIA_ERROR_ACTION (*MDT_MEDIA_ERROR_HANDLER_FN)(unsigned int iErrorCode);

////////////////////////////////////////////////////////////////////////////////
class CMdt_Media_BASE
////////////////////////////////////////////////////////////////////////////////
//
// This is our base class from which all Media Classes shall derive.
{
public:
	enum MEDIA_DIRECTION	{ MD_UNKOWN, MD_IN, MD_OUT, MD_IN_AND_OUT };
	enum MEDIA_TYPE			{ MT_UNKOWN, MT_RAWGLPIC, MT_IMGSEQUENCE, MT_AVI };

	CMdt_Media_BASE();															// constructor, this one will set media direction and type to unknown
	CMdt_Media_BASE(MEDIA_TYPE eMediaType, MEDIA_DIRECTION eMediaDirection);	// constructor, will init type and direction

	MEDIA_DIRECTION	GetMediaDirection();
	MEDIA_TYPE		GetMediaType();

	unsigned int				GetLastUnhandledError(); // calling this will return the last unhandled error (or 0 / no error if there was none) and will set it to 0 (no error) again, so only if a new error occurs it will be different from 0
	MDT_MEDIA_ERROR_HANDLER_FN	pOnErrorHandler; // you can use this to hook an error handler (in this case the class won't collect Errors anymore)

	MDT_MEDIA_ERROR_ACTION		_OnErrorHandler(unsigned int iErrorCode); // I want other classes to be able to acces this without being a friend :]
//	friend class CMdt_Media_RAWGLPIC;

private:
	MEDIA_DIRECTION	_media_direction;
	MEDIA_TYPE		_media_type;

	unsigned int	_last_unhandled_error;
	bool			_has_unhandled_error;

	void			_CMdt_Media_BASE(); // this is to share some stuff between different constructors

	void			SetMediaDirection (MEDIA_DIRECTION eMediaDirection);
	void			SetMediaType(MEDIA_TYPE eMediaType);

};

////////////////////////////////////////////////////////////////////////////////
class CMdt_Media_RAWGLPIC : public CMdt_Media_BASE
////////////////////////////////////////////////////////////////////////////////
//
// This is a very simple class in order to organize a buffer for GL data.
// It automatically adjusts buffersizes etc..
//
// How to use:
//   Simply call DoGlReadPixels with your needs, the class will try to adjust everything as far as possible.
//   The data and service functions can be trusted if DoGlReadPixels returns with true (it also set's HasConsistentData to true in this case).
{
private:
	void swap(CMdt_Media_RAWGLPIC& o);

public:
	CMdt_Media_RAWGLPIC(); // Constructor, use this one if you want to only use autodetection
	~CMdt_Media_RAWGLPIC();	// destructor
	CMdt_Media_RAWGLPIC(const CMdt_Media_RAWGLPIC& o);
	CMdt_Media_RAWGLPIC(CMdt_Media_RAWGLPIC&& o);
	CMdt_Media_RAWGLPIC& operator=(CMdt_Media_RAWGLPIC rhs);

	// glReadPixels - we always use glCoords here

	// bRepack - DoGlReadPixels assumes GL_PACKALIGNMENT to be 4. if true, it will repack the data in memory and remove spaces (good 4 tga raw output, bad 4 raw bitmap)
	bool			DoGlReadPixels(int iXofs, int iYofs, int iWidth, int iHeight, GLenum eGLformat, GLenum eGLtype, bool bRepack);
//	void			RepeatGlReadPixels();				  // equals DoGlReadPixels(0,0)
//	void			RepeatGlReadPixels(int iXofs, int iYofs); // performfs glReadPixels with an optional offset and the params you have set when this class was created

	bool	HasConsistentData();	// if this is true you can expect the class to contain valid data from an open gl buffer

	bool	CompactStructures();	// will try to release unused Memory (if more is allocated than needed) (it might fail and HasConsistentData might get false). Calling memory management to consumes time and adds to memory fragmentation, should only be done when usefull.
	void	FreeStructures();		// will release every data / memory allocated, HasConsistentData() will be false for sure.

	// ServiceFunctions:
	// The following functions may return random data and shouldn't be called when bContainsValidData()==false
	int				GetWidth();		// returns the width in pixels
	int				GetHeight();	// teturns the height in pixels

	unsigned int	GetSize();		// returns the total size in bytes of the buffer structure
	unsigned char*	GetPointer();	// returns the pointer to the buffer structure

	unsigned short	GetPixelSize();		// returns the size of the data structure in bytes for one pixel
	unsigned char	GetNumComponents();	// returns the number of components per pixel
	unsigned char	GetSizeComponent();	// returns the size of one component in bytes

	GLenum			GetGLFormat();	// returns the GLformat
	GLenum			GetGLType();	// returns the GLtype
private:
	bool			_bHasConsistentData; // if things can be trusted (last operation did not fail)
	int				_iWidth;	// width in pixels
	int				_iHeight;	// height in pixels

	unsigned int	_uiBytesAllocated;	// currently allocated bytes of memory
	unsigned int	_uiSize;			// total structure size in bytes

	unsigned short	_uiPixelSize;		// size of datatype for one pixel in bytes
	unsigned char	_ucSizeComponent;	// size of one component of a pixel in bytes
	unsigned char	_ucNumComponents;	// number of components a pixel has, also see OGL refrence for glReadPixels
	bool			_bComponentIsSigned;	//

	unsigned char*	_pBuffer;	// pointer to the buffer in memory

	GLenum			_eGLformat; // stores the GLformat
	GLenum			_eGLtype;	// stores the GLtype

	bool			_CalcSizeComponent (GLenum eGLtype, unsigned char* outSizeComponent, bool* outIsSigned);	// returns the size of one component in bytes if result is true, false if unknown, also used by constructor
	bool			_CalcNumComponents (GLenum eGLformat, unsigned char* outNumComponents);	// returns false if unknown otherwise true and the number of components per pixel

	bool			_adjustMemory(unsigned int iNewSize,bool bOnlyWhenGreater); // expands the memory if needed, returns true on succes, calls errorhandler if it fails, tries to Compact instantly if  bOnlyWhenGreater==false;
	bool			_compactMemory();						// tries to free memory if not everything is used, calls errorhandler if it fails
	void			_freeAndClean();						// frees memory and cleans up structures
};


#endif
/*
File        : mdt_gltools.h
Started     : 2007-008-02 11:56:21
Project     : Mirv Demo Tool
Authors     : Dominik Tugend
Description : various tools for handling OpenGL related stuff
*/

// WARNING THIS CLASS IS STILL IN DEVELOPEMENT STAGE AND HAS STILL TO BE TESTED BY ME

//
//  OpenGL Read and Draw buffer handling
//  this should be moved to some own file or class, but I am not sure yet which would be a good location or class, may be we should move all openGL messing stuff into one class (and cpp and h file), which should be more some sort of wrapper, meaning other classes are registered in some sort of very simple message loop (either returning allowing others to handle it or singaling no other function should handle it) for each funktion the hook

#ifndef MDT_GLTOOLS_H
#define MDT_GLTOOLS_H

#include <windows.h>
//#include <winbase.h>
#include <gl\gl.h>

#define SIZE_Mdt_GlTools_GlBuffs	14
#define MDT_GLTOOLS_BUFFER_UNKOWN	SIZE_Mdt_GlTools_GlBuffs
// pay attention that the code relays on the unkown thing at the end (which needs an additional array slot):
extern const GLint iMdt_GlTools_GlBuffNumbers [ SIZE_Mdt_GlTools_GlBuffs + 1]; // defined in mdt_gltools.cpp
extern const char* cMdt_GlTools_GlBuffStrings [ SIZE_Mdt_GlTools_GlBuffs + 1]; // defined in mdt_gltools.cpp

class cMdt_GlTools
{
private:
	int   _m_iForcedReadNum;  // this stores the index in the array of the currently selected buffer to allow to look the string up again
	int   _m_iForcedDrawNum;  // .
	GLint _m_iForcedReadBuff; // this stores the type of the currently selected buffer to avoid table lookups
	GLint _m_iForcedDrawBuff; // .

	bool _SetBufferFromStr(const char* pString,bool bIsReadBuff); // if bIsReadBuff then sets the ReadBuff, WriteBuff otherwise. result indicates if it had succes or not
	bool _giveBufferFromString (const char* pString, GLint* iGlBuff,int* arrayapos); // on false results are ondefined on return true iGLBuff returns the GLBuffertype and arrayapos returns the slot of the string / type in our arrays

public:
	// vars used for handling forcing of Read and Draw buffers:
	bool   m_bForceReadBuff;  // default see constructor, this tells functions in Auto mode if they shall aplly the buffer setting or not
	bool   m_bForceDrawBuff;  // .

	// vars for retriving currently selected GL buffers:
	GLint m_iSavedReadBuff;  // the contents are only valid when a function has been called with the request for saving the old ones
	GLint m_iSavedDrawBuff;  // .

	// constructor:
	cMdt_GlTools()
	{
		m_bForceReadBuff = true; // by default we force
		m_bForceDrawBuff = true; // by default we force

		m_iSavedReadBuff  = GL_NONE; // actually we should it leave undefined, but u never know heh
		m_iSavedDrawBuff  = GL_NONE; // .

		// private inits:
		_m_iForcedReadNum  = 6; // GL_BACK
		_m_iForcedReadBuff = iMdt_GlTools_GlBuffNumbers[_m_iForcedReadNum];
		_m_iForcedDrawNum  = 6; // GL_BACK
		_m_iForcedDrawBuff = iMdt_GlTools_GlBuffNumbers[_m_iForcedDrawNum];
	}

	// functions to controll the buffers we want to force in our class:
	bool SetReadBufferFromStr(const char* pString); // use this to set preffered buffers, returns true on success, false otherwise. if it fails the request is ignored
	bool SetDrawBufferFromStr(const char* pString); // .
	void SetReadBufferUnkown (GLenum iGlBuffer);    // this is a faster version that can also set any buffer, however GetXBufferStr functions will return "UNKNOWN"
	void SetDrawBufferUnkown (GLenum iGlBuffer);    // .

	GLenum GetReadBuffer();    // returns type of currently selected buffer
	GLenum GetDrawBuffer();    // .
	const char* GetReadBufferStr(); // might return "UNKOWN" if a buffer was set with SetXBufferUnkown
	const char* GetDrawBufferStr(); // .

	// functions to tell GL to apply the buffers we want:
    void AdjustReadBuffer(bool bAuto = true);                // if bAuto is false then it forces m_eForcedReadBuff otherwise it only forces if m_bForceReadBuff is true (default)
	void AdjustReadBuffer(GLenum eGlBuff,bool bAuto = true); // similar, except it works with the eGlBuff instead of the internal one
	void AdjustDrawBuffer(bool bAuto = true);                // .
	void AdjustDrawBuffer(GLenum eGlBuff,bool bAuto = true); // .

	// functions for retriving 
	void SaveReadBuffer(); // saves currently selected (GL!! NOT OUR CLASS) ReadBuffer to m_eSavedReadBuff
	void SaveDrawBuffer(); // saves currently selected (GL!! NOT OUR CLASS) DrawBuffer to m_eSavedDrawBuff

	// misc functions:
	bool giveGlBufferFromString (const char* pString, GLint* iGlBuff); // returns the GL Buffer Type for a given string in iGlBuff if on true, on false returns are undefined, the current implementation is not very fast actually

}; // <-- notice the ; !!!

//
// global g_Mdt_GlTools:
//
extern cMdt_GlTools g_Mdt_GlTools; // defined in mdt_gltools.cpp // our global tools suitcase :)

#endif
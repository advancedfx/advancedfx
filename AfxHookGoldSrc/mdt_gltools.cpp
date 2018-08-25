/*
File        : mdt_gltools.cpp
Started     : 2007-008-02 11:56:21
Project     : Mirv Demo Tool
Description : see mdt_gltools.h
*/

#include "mdt_gltools.h"

// pay attention that the code relays on the unkown thing at the end (which needs an additional array slot):
const GLint iMdt_GlTools_GlBuffNumbers [ SIZE_Mdt_GlTools_GlBuffs + 1] = { GL_NONE,  GL_FRONT_LEFT,  GL_FRONT_RIGHT,  GL_BACK_LEFT,  GL_BACK_RIGHT ,  GL_FRONT,  GL_BACK,  GL_LEFT,  GL_RIGHT, GL_FRONT_AND_BACK,  GL_AUX0,  GL_AUX1,  GL_AUX2,  GL_AUX3,  GL_NONE  }; // buff number for index
const char* cMdt_GlTools_GlBuffStrings [ SIZE_Mdt_GlTools_GlBuffs + 1] = {"GL_NONE","GL_FRONT_LEFT","GL_FRONT_RIGHT","GL_BACK_LEFT","GL_BACK_RIGHT","GL_FRONT","GL_BACK","GL_LEFT","GL_RIGHT","GL_FRONT_AND_BACK","GL_AUX0","GL_AUX1","GL_AUX2","GL_AUX3", "UNKOWN" }; // this array holds the Names of GlBuffers we know


void cMdt_GlTools::AdjustReadBuffer(bool bAuto)
{
	AdjustReadBuffer(_m_iForcedReadBuff,bAuto);
}
void cMdt_GlTools::AdjustReadBuffer(GLenum eGlBuff,bool bAuto)
{
	if (!bAuto || m_bForceReadBuff) glReadBuffer(eGlBuff);
}
void cMdt_GlTools::AdjustDrawBuffer(bool bAuto)
{
	AdjustDrawBuffer(_m_iForcedDrawBuff,bAuto);
}
void cMdt_GlTools::AdjustDrawBuffer(GLenum eGlBuff,bool bAuto)
{
	if (!bAuto || m_bForceDrawBuff) glDrawBuffer(eGlBuff);
}

void cMdt_GlTools::SaveReadBuffer()
{
	glGetIntegerv(GL_READ_BUFFER,&m_iSavedReadBuff);
}

void cMdt_GlTools::SaveDrawBuffer()
{
	glGetIntegerv(GL_DRAW_BUFFER,&m_iSavedDrawBuff);
}


bool cMdt_GlTools::_giveBufferFromString (const char* pString, GLint* iGlBuff,int* arrayapos)
{
	// this is currently very slow, may be s.o. can optimize it:

	for (int i=0;i<SIZE_Mdt_GlTools_GlBuffs;i++)
		if (!stricmp(cMdt_GlTools_GlBuffStrings[i], pString))
		{
			// success we found it:
			*arrayapos = i;                            // return array index
			*iGlBuff   = iMdt_GlTools_GlBuffNumbers[i]; // return Typenumber
			return true; // get out of here
		};
	;

	return false; // did not find it
}

bool cMdt_GlTools::giveGlBufferFromString (const char* pString, GLint* iGlBuff)
{
	int dummy;
	return _giveBufferFromString(pString,iGlBuff,&dummy);
}

bool cMdt_GlTools::_SetBufferFromStr(const char* pString,bool bIsReadBuff)
{
	int arraypos;
	GLint iGlPos;
	bool mres = _giveBufferFromString(pString,&iGlPos,&arraypos);

	if (mres)
	{
		// we are fine to set everything:
		if (bIsReadBuff)
		{
			_m_iForcedReadNum      =  arraypos;
			_m_iForcedReadBuff     =  iGlPos;
		} else {
			_m_iForcedDrawNum      =  arraypos;
			_m_iForcedDrawBuff     =  iGlPos;
		}
	}

	return mres;
}

bool cMdt_GlTools::SetReadBufferFromStr(const char* pString)
{
	return _SetBufferFromStr(pString,true);
}
bool cMdt_GlTools::SetDrawBufferFromStr(const char* pString)
{
	return _SetBufferFromStr(pString,false);
}

void cMdt_GlTools::SetReadBufferUnkown (GLenum iGlBuffer)
{
	_m_iForcedReadNum  = MDT_GLTOOLS_BUFFER_UNKOWN;
	_m_iForcedReadBuff = iGlBuffer;
}

void cMdt_GlTools::SetDrawBufferUnkown (GLenum iGlBuffer)
{
	_m_iForcedDrawNum  = MDT_GLTOOLS_BUFFER_UNKOWN;	
	_m_iForcedDrawBuff = iGlBuffer;
}


GLenum cMdt_GlTools::GetReadBuffer()
{
	return _m_iForcedReadBuff;
}
GLenum cMdt_GlTools::GetDrawBuffer()
{
	return _m_iForcedDrawBuff;
}
const char* cMdt_GlTools::GetReadBufferStr()
{
	return cMdt_GlTools_GlBuffStrings[_m_iForcedReadNum];
}

const char* cMdt_GlTools::GetDrawBufferStr()
{
	return cMdt_GlTools_GlBuffStrings[_m_iForcedDrawNum];
}

//
// global g_Mdt_GlTools:
//
cMdt_GlTools g_Mdt_GlTools; // defined in mdt_gltools.cpp // our global tools suitcase :)

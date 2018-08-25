#include "stdafx.h"

#include "GlPrimMods.h"

#include "mirv_glext.h"


using namespace GlPrimMod;

inline GLboolean GlPrimMod::ToGLboolean(ChannelMode value, GLboolean oldGlMode)
{
	return
		CM_NoChange == value ? oldGlMode : (
			CM_Enabled == value ? GL_TRUE : GL_FALSE
		)
	;
}

inline int GlPrimMod::ToInt(ChannelMode value)
{
	return
		CM_NoChange == value ? 0 : (
			CM_Enabled == value ? 1 : -1
		)
	;
}

inline ChannelMode GlPrimMod::FromInt(int value)
{
	return
		0 == value ? CM_NoChange : (
			0 < value ? CM_Enabled : CM_Disabled
		)
	;
}


// Color ///////////////////////////////////////////////////////////////////////

Color::Color()
{
	m_Alpha = -1;
	m_Blue = -1;
	m_Green = -1;
	m_Red = -1;
}

GLfloat Color::GetAlpha() {
	return m_Alpha;
}

GLfloat Color::GetBlue() {
	return m_Blue;
}

GLfloat Color::GetGreen() {
	return m_Green;
}

GLfloat Color::GetRed() {
	return m_Red;
}

void Color::OnGlBegin(GLenum mode)
{
	glGetFloatv(GL_CURRENT_COLOR, m_Old_Gl_Color);

	glColor4f(
		(0 <= m_Red ? m_Red : m_Old_Gl_Color[0]),
		(0 <= m_Green ? m_Green : m_Old_Gl_Color[1]),
		(0 <= m_Blue ? m_Blue : m_Old_Gl_Color[2]),
		(0 <= m_Alpha ? m_Alpha : m_Old_Gl_Color[3])
	);
}

void Color::OnGlEnd()
{
	glColor4f(m_Old_Gl_Color[0], m_Old_Gl_Color[1], m_Old_Gl_Color[2], m_Old_Gl_Color[3]);
}

void Color::SetAlpha(GLfloat value)
{
	m_Alpha = value;
}

void Color::SetBlue(GLfloat value)
{
	m_Blue = value;
}

void Color::SetGreen(GLfloat value)
{
	m_Green = value;
}

void Color::SetRed(GLfloat value)
{
	m_Red = value;
}


void Color::SetRgba(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	m_Alpha = alpha;
	m_Blue = blue;
	m_Green = green;
	m_Red = red;
}



// ColorMask //////////////////////////////////////////////////////////

ColorMask::ColorMask()
{
	m_Alpha = CM_NoChange;
	m_Blue = CM_NoChange;
	m_Green = CM_NoChange;
	m_Red = CM_NoChange;
}

ChannelMode ColorMask::GetAlpha()
{
	return m_Alpha;
}

ChannelMode ColorMask::GetBlue()
{
	return m_Blue;
}

ChannelMode ColorMask::GetGreen()
{
	return m_Green;
}

ChannelMode ColorMask::GetRed()
{
	return m_Red;
}


void ColorMask::OnGlBegin(GLenum mode)
{
	glGetBooleanv(GL_COLOR_WRITEMASK, m_Old_Gl_ColorMask);

	glColorMask(
		ToGLboolean(m_Red, m_Old_Gl_ColorMask[0]), 
		ToGLboolean(m_Green, m_Old_Gl_ColorMask[1]),
		ToGLboolean(m_Blue, m_Old_Gl_ColorMask[2]),
		ToGLboolean(m_Alpha, m_Old_Gl_ColorMask[3])
	);
}

void ColorMask::OnGlEnd()
{
	glColorMask(m_Old_Gl_ColorMask[0], m_Old_Gl_ColorMask[1], m_Old_Gl_ColorMask[2], m_Old_Gl_ColorMask[3]);
}

void ColorMask::SetAlpha(ChannelMode value)
{
	m_Alpha = value;
}

void ColorMask::SetBlue(ChannelMode value)
{
	m_Blue = value;
}

void ColorMask::SetGreen(ChannelMode value)
{
	m_Green = value;
}

void ColorMask::SetRed(ChannelMode value)
{
	m_Red = value;
}


void ColorMask::SetRgba(ChannelMode red, ChannelMode green, ChannelMode blue, ChannelMode alpha)
{
	m_Alpha = alpha;
	m_Blue = blue;
	m_Green = green;
	m_Red = red;
}


// DepthMask //////////////////////////////////////////////////////////

DepthMask::DepthMask()
{
	m_Depth = CM_NoChange;
}

ChannelMode DepthMask::GetDepth()
{
	return m_Depth;
}


void DepthMask::OnGlBegin(GLenum mode)
{
	glGetBooleanv(GL_DEPTH_WRITEMASK, &m_Old_Depth);

	glDepthMask(
		ToGLboolean(m_Depth, m_Old_Depth)
	);
}

void DepthMask::OnGlEnd()
{
	glDepthMask(m_Old_Depth);
}

void DepthMask::SetDepth(ChannelMode value)
{
	m_Depth = value;
}


// Mask //////////////////////////////////////////////////////////

Mask::Mask()
{
	m_Mask = CM_NoChange;
}


ChannelMode Mask::GetMask()
{
	return m_DepthMask.GetDepth();
}

void Mask::OnGlBegin(GLenum mode)
{
	m_ColorMask.OnGlBegin(mode);
	m_DepthMask.OnGlBegin(mode);
}

void Mask::OnGlEnd()
{
	m_DepthMask.OnGlEnd();
	m_ColorMask.OnGlEnd();
}

void Mask::SetMask(ChannelMode value)
{
	m_ColorMask.SetAlpha(value);
	m_ColorMask.SetRed(value);
	m_ColorMask.SetBlue(value);
	m_ColorMask.SetGreen(value);
	m_DepthMask.SetDepth(value);
}




// Replace /////////////////////////////////////////////////////////////////////


Replace::Replace()
{
	m_Blue = 0;
	m_Green = 0;
	m_HasTexture = false;
	m_Red = 0;
}

Replace::~Replace()
{
	if(m_HasTexture)
	{
		glDeleteTextures(1, &m_GlTexture);
	}
}

void Replace::EnsureTexture()
{
	if(!g_Has_GL_ARB_multitexture) return;

	if(!m_HasTexture)
	{
		glGenTextures(1, &m_GlTexture);

		m_HasTexture = true;
		m_RefreshTexture = true;
	}

	if(m_RefreshTexture)
	{
		GLint oldtex;
		GLubyte texmem[48];

		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtex);
		
		for(int i=0; i<16; i++)
		{
			texmem[3*i  ] = m_Red;
			texmem[3*i+1] = m_Green;
			texmem[3*i+2] = m_Blue;
		}

		glBindTexture(GL_TEXTURE_2D, m_GlTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, texmem);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, oldtex);

		m_RefreshTexture = false;
	}
}


GLubyte Replace::GetBlue()
{
	return m_Blue;
}

GLubyte Replace::GetGreen()
{
	return m_Green;
}

GLubyte Replace::GetRed()
{
	return m_Red;
}

bool Replace::IsSupported()
{
	return g_Has_GL_ARB_multitexture;
}


void Replace::OnGlBegin(GLenum mode)
{
	EnsureTexture();

	if(!m_HasTexture) return;


	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &m_Old_Gl_Active_Texture_Arb);

	glActiveTextureARB(GL_TEXTURE2_ARB);

	m_Old_Gl_Texture2d = glIsEnabled(GL_TEXTURE_2D);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_Old_Gl_TextureBinding2d);
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &m_Old_Gl_Texture_Env_Mode);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_GlTexture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void Replace::OnGlEnd()
{
	if(!m_HasTexture) return;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_Old_Gl_Texture_Env_Mode);
	glBindTexture(GL_TEXTURE_2D, m_Old_Gl_TextureBinding2d);
	if(!m_Old_Gl_Texture2d) glDisable(GL_TEXTURE_2D);

	glActiveTextureARB(m_Old_Gl_Active_Texture_Arb);
}


void Replace::SetBlue(GLubyte value)
{
	m_Blue = value;
	m_RefreshTexture = true;
}

void Replace::SetGreen(GLubyte value)
{
	m_Green = value;
	m_RefreshTexture = true;
}

void Replace::SetRed(GLubyte value)
{
	m_Red = value;
	m_RefreshTexture = true;
}

void Replace::SetRgb(GLubyte red, GLubyte green, GLubyte blue)
{
	m_Blue = blue;
	m_Green = green;
	m_Red = red;
	m_RefreshTexture = true;
}



// Stack /////////////////////////////////////////////////////////////

void Stack::AddAt(IGlPrimMod * glPrimMod, int index)
{
	m_GlPrimMods.insert(
		m_GlPrimMods.begin() +index,
		glPrimMod
	);
}


IGlPrimMod * Stack::GetAt(int index)
{
	return m_GlPrimMods[index];
}

int Stack::GetCount()
{
	return m_GlPrimMods.size();
}

void Stack::OnGlBegin(GLenum mode)
{
	for(
		std::vector<IGlPrimMod *>::iterator it = m_GlPrimMods.begin();
		it != m_GlPrimMods.end();
		it++)
	{
		(*it)->OnGlBegin(mode);
	}
}

void Stack::OnGlEnd()
{
	for(
		std::vector<IGlPrimMod *>::iterator it = m_GlPrimMods.begin();
		it != m_GlPrimMods.end();
		it++)
	{
		(*it)->OnGlEnd();
	}
}


void Stack::Remove(IGlPrimMod * glPrimMod)
{
	for(
		std::vector<IGlPrimMod *>::iterator it = m_GlPrimMods.begin();
		it != m_GlPrimMods.end();
		it++
	) {
		if(*it == glPrimMod)
		{
			m_GlPrimMods.erase(it);
			break;
		}
	}
}


IGlPrimMod * Stack::RemoveAt(int index)
{
	std::vector<IGlPrimMod *>::iterator it = m_GlPrimMods.begin() +index;
	IGlPrimMod * result = *it;

	m_GlPrimMods.erase(it);

	return result;
}


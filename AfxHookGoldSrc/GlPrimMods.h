#pragma once

// Description:
//
// Per OpenGl primitive modifiers.

#include <windows.h>
#include <gl\gl.h>

#include <vector>


namespace GlPrimMod {

class Color;
class ColorMask;
class DepthMask;
class IGlPrimMod;
class Mask;
class Replace;
class Stack;

enum ChannelMode
{
	CM_NoChange = 0,
	CM_Enabled,
	CM_Disabled
};


extern inline GLboolean ToGLboolean(ChannelMode value, GLboolean oldGlMode);
extern inline int ToInt(ChannelMode value);
extern inline ChannelMode FromInt(int value);


// IGlPrimMod //////////////////////////////////////////////////////////////////

class __declspec(novtable) IGlPrimMod abstract
{
public:

	virtual void OnGlBegin(GLenum mode) abstract = 0;

	virtual void OnGlEnd() abstract = 0;
};


// Color ///////////////////////////////////////////////////////////////////////

class Color :
	public IGlPrimMod
{
public:
	Color();

	GLfloat GetAlpha();
	GLfloat GetBlue();
	GLfloat GetGreen();
	GLfloat GetRed();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	/// <remarks>value &lt; 0 indicates NoChange</remarks>
	void SetAlpha(GLfloat value);

	/// <remarks>value &lt; 0 indicates NoChange</remarks>
	void SetBlue(GLfloat value);

	/// <remarks>value &lt; 0 indicates NoChange</remarks>
	void SetGreen(GLfloat value);

	/// <remarks>value &lt; 0 indicates NoChange</remarks>
	void SetRed(GLfloat value);

	/// <remarks>value &lt; 0 indicates NoChange</remarks>
	void SetRgba(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

private:
	GLfloat m_Alpha;
	GLfloat m_Blue;
	GLfloat m_Green;
	GLfloat m_Old_Gl_Color[4];
	GLfloat m_Red;
};


// ColorMask ///////////////////////////////////////////////////////////////////

class ColorMask :
	public IGlPrimMod
{
public:
	ColorMask();

	ChannelMode GetAlpha();
	ChannelMode GetBlue();
	ChannelMode GetGreen();
	ChannelMode GetRed();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	void SetAlpha(ChannelMode value);
	void SetBlue(ChannelMode value);
	void SetGreen(ChannelMode value);
	void SetRed(ChannelMode value);

	void SetRgba(ChannelMode red, ChannelMode green, ChannelMode blue, ChannelMode alpha);

private:
	ChannelMode m_Alpha;
	ChannelMode m_Blue;
	ChannelMode m_Green;
	GLboolean m_Old_Gl_ColorMask[4];
	ChannelMode m_Red;
};


// DepthMask ///////////////////////////////////////////////////////////////////

class DepthMask :
	public IGlPrimMod
{
public:
	DepthMask();

	ChannelMode GetDepth();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	void SetDepth(ChannelMode value);


private:
	ChannelMode m_Depth;
	GLboolean m_Old_Depth;
};


// Mask ////////////////////////////////////////////////////////////////////////

class Mask :
	public IGlPrimMod
{
public:
	Mask();

	ChannelMode GetMask();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	void SetMask(ChannelMode value);


private:
	ColorMask m_ColorMask;
	DepthMask m_DepthMask;
	ChannelMode m_Mask;
};


// Replace /////////////////////////////////////////////////////////////////////

class Replace :
	public IGlPrimMod
{
public:
	Replace();
	~Replace();

	GLubyte GetBlue();
	GLubyte GetGreen();
	GLubyte GetRed();

	static bool IsSupported();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	/// <remarks>Changing the color causes the texture to be regenerated, using multiple
	/// Replace instances (and thus textures) is usually prefered.</remarks>
	void SetBlue(GLubyte value);

	/// <remarks>Changing the color causes the texture to be regenerated, using multiple
	/// Replace instances (and thus textures) is usually prefered.</remarks>
	void SetGreen(GLubyte value);

	/// <remarks>Changing the color causes the texture to be regenerated, using multiple
	/// Replace instances (and thus textures) is usually prefered.</remarks>
	void SetRed(GLubyte value);

	/// <remarks>Changing the color causes the texture to be regenerated, using multiple
	/// Replace instances (and thus textures) is usually prefered.</remarks>
	void SetRgb(GLubyte red, GLubyte green, GLubyte blue);


private:
	GLubyte m_Blue;
	GLuint m_GlTexture;
	GLubyte m_Green;
	bool m_HasTexture;
	GLint m_Old_Gl_Active_Texture_Arb;
	GLboolean m_Old_Gl_Texture2d;
	GLint m_Old_Gl_Texture_Env_Mode;
	GLint m_Old_Gl_TextureBinding2d;
	GLubyte m_Red;
	bool m_RefreshTexture;

	void EnsureTexture();
};


// Stack ///////////////////////////////////////////////////////////////////////

/// <summary>A stack of IGlPrimMods.</summary>
class Stack :
	public IGlPrimMod
{
public:
	void AddAt(IGlPrimMod * glPrimMod, int index);

	IGlPrimMod * GetAt(int index);

	int GetCount();

	/// <summary>Implements IGlPrimMod.OnGlBegin.</summary>
	virtual void OnGlBegin(GLenum mode);

	/// <summary>Implements IGlPrimMod.OnGlEnd.</summary>
	virtual void OnGlEnd();

	void Remove(IGlPrimMod * glPrimMod);
	IGlPrimMod * RemoveAt(int index);

private:
	std::vector<IGlPrimMod *> m_GlPrimMods;
};

} // namespace GlPrimMod {

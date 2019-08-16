#pragma once

#include <d3d9.h>

#include <map>
#include <string>

class IAfxVertexShader abstract
{
public:
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual IDirect3DVertexShader9 * GetVertexShader() = 0;
};

class IAfxPixelShader abstract
{
public:
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual IDirect3DPixelShader9 * GetPixelShader() = 0;
};

class CAfxShaders;

class CAfxShader
{
public:
	CAfxShader();

	virtual void AddRef();
	virtual void Release();

	unsigned long GetRefCount();

protected:
	virtual ~CAfxShader();

private:
	unsigned long m_RefCount;
};

class CAfxVertexShader
: public CAfxShader
, public IAfxVertexShader
{
public:
	CAfxVertexShader();

	virtual void AddRef();
	virtual void Release();

	virtual IDirect3DVertexShader9 * GetVertexShader();

	void BeginDevice(IDirect3DDevice9 * device, const wchar_t * name);
	void EndDevice();

protected:
	virtual ~CAfxVertexShader();

private:
	IDirect3DVertexShader9 * m_VertexShader;
};

class CAfxPixelShader
: public CAfxShader
, public IAfxPixelShader
{
public:
	CAfxPixelShader();

	virtual void AddRef();
	virtual void Release();

	virtual IDirect3DPixelShader9 * GetPixelShader();

	void BeginDevice(IDirect3DDevice9 * device, const wchar_t * name);
	void EndDevice();

protected:
	virtual ~CAfxPixelShader();

private:
	IDirect3DPixelShader9 * m_PixelShader;

};

class CAfxAcsVertexShader
: public CAfxShader
, public IAfxVertexShader
{
public:
	CAfxAcsVertexShader();

	virtual void AddRef();
	virtual void Release();

	virtual IDirect3DVertexShader9 * GetVertexShader();

	void BeginDevice(IDirect3DDevice9 * device, const wchar_t * name, int combo);
	void EndDevice();

protected:
	virtual ~CAfxAcsVertexShader();

private:
	IDirect3DVertexShader9 * m_VertexShader;

};

class CAfxAcsPixelShader
: public CAfxShader
, public IAfxPixelShader
{
public:
	CAfxAcsPixelShader();

	virtual void AddRef();
	virtual void Release();

	virtual IDirect3DPixelShader9 * GetPixelShader();

	void BeginDevice(IDirect3DDevice9 * device, const wchar_t * name, int combo);
	void EndDevice();

protected:
	virtual ~CAfxAcsPixelShader();

private:
	IDirect3DPixelShader9 * m_PixelShader;

};

class CAfxShaders
{
public:
	CAfxShaders();
	~CAfxShaders();

	IAfxVertexShader * GetVertexShader(const wchar_t * name);
	
	IAfxPixelShader * GetPixelShader(const wchar_t * name);

	IAfxVertexShader * GetAcsVertexShader(const wchar_t * name, int combo);
	IAfxPixelShader * GetAcsPixelShader(const wchar_t * name, int combo);

	void BeginDevice(IDirect3DDevice9 * device);
	void EndDevice();

	void ReleaseUnusedShaders();

private:
	class CAcsShaderKey
	{
	public:
		std::wstring Name;
		int Combo;

		CAcsShaderKey();
		CAcsShaderKey(const wchar_t * name, int combo);

		CAcsShaderKey(const CAcsShaderKey & x);
		
		bool operator < (const CAcsShaderKey & y) const;
	};

	IDirect3DDevice9 * m_Device;
	std::map<std::wstring,CAfxVertexShader *> m_VertexShaders;
	std::map<std::wstring,CAfxPixelShader *> m_PixelShaders;
	std::map<CAcsShaderKey,CAfxAcsVertexShader *> m_AcsVertexShaders;
	std::map<CAcsShaderKey,CAfxAcsPixelShader *> m_AcsPixelShaders;
};

extern CAfxShaders g_AfxShaders;

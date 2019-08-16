#include "stdafx.h"

#include "AfxShaders.h"

#include "hlaeFolder.h"
//#include "SourceInterfaces.h"

#include <string>

CAfxShaders g_AfxShaders;

bool GetShaderDirectory(std::wstring & outShaderDirectory)
{
	outShaderDirectory.assign(GetHlaeFolderW());

	outShaderDirectory.append(L"resources\\AfxHookSource\\shaders\\");

	return true;
}

DWORD * LoadShaderFileInMemory(const wchar_t * fileName)
{
	if(!fileName)
		return 0;

	std::wstring shaderDir;

	if(!GetShaderDirectory(shaderDir))
		return 0;

	shaderDir.append(fileName);

	FILE * file = 0;
	bool bOk = 0 == _wfopen_s(&file, shaderDir.c_str(), L"rb");
	DWORD * so = 0;
	size_t size = 0;

	bOk = bOk 
		&& 0 != file
		&& 0 == fseek(file, 0, SEEK_END)
	;

	if(bOk)
	{
		size = ftell(file);

		so = (DWORD *)malloc(
			(size & 0x3) == 0 ? size : size +(4-(size & 0x3))
		);
		bOk = 0 != so
			&& 0 == fseek(file, 0, SEEK_SET);
	}

	if(bOk)
	{
		bOk = size == fread(so, 1, size, file);
	}

	if(file) fclose(file);

	if(bOk)
		return so;
	
	if(so) free(so);
	
	return 0;
}

int FindAcsIndex(FILE * file, size_t indexOfs, int lowIndex, int hiIndex, int combo)
{
	if(hiIndex < lowIndex)
		return -1;

	int testIndex = (lowIndex +hiIndex) >> 1;

	if(0 != fseek(file, indexOfs + testIndex * (sizeof(int) + sizeof(int)), SEEK_SET))
		return -1;

	int value;

	if(1 != fread(&value,sizeof(value),1,file))
		return -1;

	if(value == combo)
	{
		return testIndex;
	}

	return combo < value
		? FindAcsIndex(file, indexOfs, lowIndex, testIndex-1, combo)
		:  FindAcsIndex(file, indexOfs, testIndex+1, hiIndex, combo)
	;
}

/// <remarks>Assumes that the file is right after the position of the combo (i.e. use after FindAcsIndex).</remarks>
bool GetAcsComboOffsetAndSize(FILE * file, int index, int indexSize, size_t & outComboOffset, size_t & outComboSize)
{
	int comboOfs;

	if(1 != fread(&comboOfs,sizeof(comboOfs),1,file))
		return false;

	if(index+1 < indexSize)
	{
		if(0 != fseek(file, sizeof(int), SEEK_CUR))
			return false;

		int nextComboOfs;

		if(1 != fread(&nextComboOfs,sizeof(nextComboOfs),1,file))
			return false;

		outComboOffset = comboOfs;
		outComboSize = nextComboOfs - comboOfs;

		return true;
	}

	// Is last combo, so filesize determines the size:
	if(0 != fseek(file, 0, SEEK_END))
		return false;

	size_t nextComboOfs = ftell(file);

	outComboOffset = comboOfs;
	outComboSize = nextComboOfs - comboOfs;

	return true;
}

DWORD * LoadFromAcsShaderFileInMemory(const wchar_t * fileName, int combo)
{
	if(!fileName)
		return 0;

	std::wstring shaderDir;

	if(!GetShaderDirectory(shaderDir))
		return 0;

	shaderDir.append(fileName);

	FILE * file = 0;
	bool bOk = 0 == _wfopen_s(&file, shaderDir.c_str(), L"rb");
	DWORD * so = 0;
	size_t size = 0;
	int version;
	int indexSize;
	size_t indexOfs;
	int index;
	size_t comboSize;
	size_t comboOfs;

	bOk = bOk 
		&& 0 != file
		&& 1 == fread(&version,sizeof(version),1,file)
		&& 0 == version
		&& 1 == fread(&indexSize,sizeof(indexSize),1,file)
		&& (indexOfs = ftell(file), true)
		&& -1 != (index = FindAcsIndex(file,indexOfs,0,indexSize-1,combo))
		&& GetAcsComboOffsetAndSize(file, index, indexSize, comboOfs, comboSize)
	;

	if(bOk)
	{
		//Tier0_Msg("Combo %i is at 0x%08x and has size 0x%08x!\n",combo,comboOfs,comboSize);

		so = (DWORD *)malloc(
			(comboSize & 0x3) == 0 ? comboSize : comboSize +(4-(size & 0x3))
		);
		bOk = 0 != so
			&& 0 == fseek(file, comboOfs, SEEK_SET)
			&& comboSize == fread(so, 1, comboSize, file)
		;
	}

	if(file) fclose(file);

	if(bOk)
		return so;
	
	if(so) free(so);
	
	return 0;
}


// CAfxVertexShader ////////////////////////////////////////////////////////////

CAfxVertexShader::CAfxVertexShader()
: m_VertexShader(0)
{
}

CAfxVertexShader::~CAfxVertexShader()
{
	if(m_VertexShader) m_VertexShader->Release();
}

void CAfxVertexShader::AddRef()
{
	CAfxShader::AddRef();
}

void CAfxVertexShader::Release()
{
	CAfxShader::Release();
}

IDirect3DVertexShader9 * CAfxVertexShader::GetVertexShader()
{
	return m_VertexShader;
}

void CAfxVertexShader::BeginDevice(IDirect3DDevice9 * device, const wchar_t * name)
{
	DWORD * so = LoadShaderFileInMemory(name);

	if(so && SUCCEEDED(device->CreateVertexShader(so, &m_VertexShader)))
		m_VertexShader->AddRef();
	else
		m_VertexShader = 0;

	if(so) free(so);
}

void CAfxVertexShader::EndDevice()
{
	if(!m_VertexShader)
		return;

	m_VertexShader->Release();
	m_VertexShader = 0;
}

// CAfxPixelShader ////////////////////////////////////////////////////////////

CAfxPixelShader::CAfxPixelShader()
: m_PixelShader(0)
{
}

CAfxPixelShader::~CAfxPixelShader()
{
	if(m_PixelShader) m_PixelShader->Release();
}

void CAfxPixelShader::AddRef()
{
	CAfxShader::AddRef();
}

void CAfxPixelShader::Release()
{
	CAfxShader::Release();
}


IDirect3DPixelShader9 * CAfxPixelShader::GetPixelShader()
{
	return m_PixelShader;
}

void CAfxPixelShader::BeginDevice(IDirect3DDevice9 * device, const wchar_t * name)
{
	DWORD * so = LoadShaderFileInMemory(name);

	if(so && SUCCEEDED(device->CreatePixelShader(so, &m_PixelShader)))
		m_PixelShader->AddRef();
	else
		m_PixelShader = 0;

	if(so) free(so);
}

void CAfxPixelShader::EndDevice()
{
	if(!m_PixelShader)
		return;

	m_PixelShader->Release();
	m_PixelShader = 0;
}

// CAfxAcsVertexShader ////////////////////////////////////////////////////////////

CAfxAcsVertexShader::CAfxAcsVertexShader()
: m_VertexShader(0)
{
}

CAfxAcsVertexShader::~CAfxAcsVertexShader()
{
	if(m_VertexShader) m_VertexShader->Release();
}

void CAfxAcsVertexShader::AddRef()
{
	CAfxShader::AddRef();
}

void CAfxAcsVertexShader::Release()
{
	CAfxShader::Release();
}


IDirect3DVertexShader9 * CAfxAcsVertexShader::GetVertexShader()
{
	return m_VertexShader;
}

void CAfxAcsVertexShader::BeginDevice(IDirect3DDevice9 * device, const wchar_t * name, int combo)
{
	DWORD * so = LoadFromAcsShaderFileInMemory(name, combo);

	if(so && SUCCEEDED(device->CreateVertexShader(so, &m_VertexShader)))
		m_VertexShader->AddRef();
	else
		m_VertexShader = 0;

	if(so) free(so);
}

void CAfxAcsVertexShader::EndDevice()
{
	if(!m_VertexShader)
		return;

	m_VertexShader->Release();
	m_VertexShader = 0;
}


// CAfxAcsPixelShader ////////////////////////////////////////////////////////////

CAfxAcsPixelShader::CAfxAcsPixelShader()
: m_PixelShader(0)
{
}

CAfxAcsPixelShader::~CAfxAcsPixelShader()
{
	if(m_PixelShader) m_PixelShader->Release();
}

void CAfxAcsPixelShader::AddRef()
{
	CAfxShader::AddRef();
}

void CAfxAcsPixelShader::Release()
{
	CAfxShader::Release();
}


IDirect3DPixelShader9 * CAfxAcsPixelShader::GetPixelShader()
{
	return m_PixelShader;
}

void CAfxAcsPixelShader::BeginDevice(IDirect3DDevice9 * device, const wchar_t * name, int combo)
{
	DWORD * so = LoadFromAcsShaderFileInMemory(name, combo);

	if(so && SUCCEEDED(device->CreatePixelShader(so, &m_PixelShader)))
		m_PixelShader->AddRef();
	else
		m_PixelShader = 0;

	if(so) free(so);
}

void CAfxAcsPixelShader::EndDevice()
{
	if(!m_PixelShader)
		return;

	m_PixelShader->Release();
	m_PixelShader = 0;
}


// CAfxShader //////////////////////////////////////////////////////////////////

CAfxShader::CAfxShader()
: m_RefCount(0)
{
}

CAfxShader::~CAfxShader()
{
}

void CAfxShader::AddRef()
{
	++m_RefCount;
}

void CAfxShader::Release()
{
	--m_RefCount;

	if(0 == m_RefCount)
		delete this;
}

unsigned long CAfxShader::GetRefCount()
{
	return m_RefCount;
}

// CAfxShaders /////////////////////////////////////////////////////////////////

CAfxShaders::CAfxShaders()
: m_Device(0)
{
}

CAfxShaders::~CAfxShaders()
{
	for(std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator it = m_AcsPixelShaders.begin(); it != m_AcsPixelShaders.end(); ++it)
	{
		it->second->Release();
	}

	for(std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator it = m_AcsVertexShaders.begin(); it != m_AcsVertexShaders.end(); ++it)
	{
		it->second->Release();
	}

	for(std::map<std::wstring,CAfxPixelShader *>::iterator it = m_PixelShaders.begin(); it != m_PixelShaders.end(); ++it)
	{
		it->second->Release();
	}

	for(std::map<std::wstring,CAfxVertexShader *>::iterator it = m_VertexShaders.begin(); it != m_VertexShaders.end(); ++it)
	{
		it->second->Release();
	}
}

IAfxVertexShader * CAfxShaders::GetVertexShader(const wchar_t * name)
{
	std::wstring sName(name);

	std::map<std::wstring,CAfxVertexShader *>::iterator it = m_VertexShaders.find(sName);
	if(it!=m_VertexShaders.end())
	{
		it->second->AddRef(); // for call
		return it->second;
	}

	CAfxVertexShader * shader = new CAfxVertexShader();
	shader->AddRef(); // for list
	shader->AddRef(); // for call

	if(m_Device) shader->BeginDevice(m_Device, sName.c_str());

	return m_VertexShaders[sName] = shader;
}

IAfxPixelShader * CAfxShaders::GetPixelShader(const wchar_t * name)
{
	std::wstring sName(name);

	std::map<std::wstring,CAfxPixelShader *>::iterator it = m_PixelShaders.find(sName);
	if(it!=m_PixelShaders.end())
	{
		it->second->AddRef(); // for call
		return it->second;
	}

	CAfxPixelShader * shader = new CAfxPixelShader();
	shader->AddRef(); // for list
	shader->AddRef(); // for call

	if(m_Device) shader->BeginDevice(m_Device, sName.c_str());

	return m_PixelShaders[sName] = shader;
}

IAfxVertexShader * CAfxShaders::GetAcsVertexShader(const wchar_t * name, int combo)
{
	CAcsShaderKey key(name, combo);

	std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator it = m_AcsVertexShaders.find(key);
	if(it!=m_AcsVertexShaders.end())
	{
		it->second->AddRef(); // for call
		return it->second;
	}

	CAfxAcsVertexShader * shader = new CAfxAcsVertexShader();
	shader->AddRef(); // for list
	shader->AddRef(); // for call

	if(m_Device) shader->BeginDevice(m_Device, key.Name.c_str(), key.Combo);

	return m_AcsVertexShaders[key] = shader;
}

IAfxPixelShader * CAfxShaders::GetAcsPixelShader(const wchar_t * name, int combo)
{
	CAcsShaderKey key(name, combo);

	std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator it = m_AcsPixelShaders.find(key);
	if(it!=m_AcsPixelShaders.end())
	{
		it->second->AddRef(); // for call
		return it->second;
	}

	CAfxAcsPixelShader * shader = new CAfxAcsPixelShader();
	shader->AddRef(); // for list
	shader->AddRef(); // for call

	if(m_Device) shader->BeginDevice(m_Device, key.Name.c_str(), key.Combo);

	return m_AcsPixelShaders[key] = shader;
}
	
void CAfxShaders::BeginDevice(IDirect3DDevice9 * device)
{
	EndDevice();

	if(0 == device)
		return;

	device->AddRef();

	m_Device = device;

	for(std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator it = m_AcsPixelShaders.begin(); it != m_AcsPixelShaders.end(); ++it)
	{
		it->second->BeginDevice(device, it->first.Name.c_str(), it->first.Combo);
	}

	for(std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator it = m_AcsVertexShaders.begin(); it != m_AcsVertexShaders.end(); ++it)
	{
		it->second->BeginDevice(device, it->first.Name.c_str(), it->first.Combo);
	}

	for(std::map<std::wstring,CAfxPixelShader *>::iterator it = m_PixelShaders.begin(); it != m_PixelShaders.end(); ++it)
	{
		it->second->BeginDevice(device, it->first.c_str());
	}

	for(std::map<std::wstring,CAfxVertexShader *>::iterator it = m_VertexShaders.begin(); it != m_VertexShaders.end(); ++it)
	{
		it->second->BeginDevice(device, it->first.c_str());
	}
}

void CAfxShaders::EndDevice()
{
	if(0 == m_Device)
		return;

	for(std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator it = m_AcsPixelShaders.begin(); it != m_AcsPixelShaders.end(); ++it)
	{
		it->second->EndDevice();
	}

	for(std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator it = m_AcsVertexShaders.begin(); it != m_AcsVertexShaders.end(); ++it)
	{
		it->second->EndDevice();
	}

	for(std::map<std::wstring,CAfxPixelShader *>::iterator it = m_PixelShaders.begin(); it != m_PixelShaders.end(); ++it)
	{
		it->second->EndDevice();
	}

	for(std::map<std::wstring,CAfxVertexShader *>::iterator it = m_VertexShaders.begin(); it != m_VertexShaders.end(); ++it)
	{
		it->second->EndDevice();
	}

	m_Device->Release();
	m_Device = 0;
}

void CAfxShaders::ReleaseUnusedShaders()
{
	for(std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator it = m_AcsPixelShaders.begin(); it != m_AcsPixelShaders.end();)
	{
		if(1 == it->second->GetRefCount())
		{
			std::map<CAcsShaderKey,CAfxAcsPixelShader *>::iterator er = it;
			++it;
			er->second->Release();
			m_AcsPixelShaders.erase(er);
		}
		else
		{
			++it;
		}
	}

	for(std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator it = m_AcsVertexShaders.begin(); it != m_AcsVertexShaders.end();)
	{
		if(1 == it->second->GetRefCount())
		{
			std::map<CAcsShaderKey,CAfxAcsVertexShader *>::iterator er = it;
			++it;
			er->second->Release();
			m_AcsVertexShaders.erase(er);
		}
		else
		{
			++it;
		}
	}

	for(std::map<std::wstring,CAfxPixelShader *>::iterator it = m_PixelShaders.begin(); it != m_PixelShaders.end();)
	{
		if(1 == it->second->GetRefCount())
		{
			std::map<std::wstring,CAfxPixelShader *>::iterator er = it;
			++it;
			er->second->Release();
			m_PixelShaders.erase(er);
		}
		else
		{
			++it;
		}
	}

	for(std::map<std::wstring,CAfxVertexShader *>::iterator it = m_VertexShaders.begin(); it != m_VertexShaders.end();)
	{
		if(1 == it->second->GetRefCount())
		{
			std::map<std::wstring,CAfxVertexShader *>::iterator er = it;
			++it;
			er->second->Release();
			m_VertexShaders.erase(er);
		}
		else
		{
			++it;
		}
	}
}

// CAfxShaders::CAcsShaderKey //////////////////////////////////////////////////

CAfxShaders::CAcsShaderKey::CAcsShaderKey()
{
}

CAfxShaders::CAcsShaderKey::CAcsShaderKey(const wchar_t * name, int combo)
: Name(name)
, Combo(combo)
{
}

CAfxShaders::CAcsShaderKey::CAcsShaderKey(const CAcsShaderKey & x)
: Name(x.Name)
, Combo(x.Combo)
{
}
		
bool CAfxShaders::CAcsShaderKey::operator < (const CAcsShaderKey & y) const
{
	int cmp = Name.compare(y.Name);

	return cmp < 0 || 0 == cmp && Combo < y.Combo;
}

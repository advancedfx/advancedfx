#pragma once

#include <shared/AfxColorLut.h>
#include <stdio.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace AfxCppCli {

public ref class ColorLutTools
{
public:
	ColorLutTools()
	{
		m_AfxColorLut = new CAfxColorLut();
	}

	~ColorLutTools()
	{
		delete m_AfxColorLut;
	}

	bool IsValid()
	{
		return m_AfxColorLut->IsValid();
	}

	bool New(size_t resR, size_t resG, size_t resB, size_t resA)
	{
		return m_AfxColorLut->New(resR, resG, resB, resA);
	}

	bool LoadFromFile(System::String^ fileName)
	{
		IntPtr pszFileName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileName);
		try
		{
			FILE* file = nullptr;
			if (0 == fopen_s(&file, static_cast<const char*>(pszFileName.ToPointer()), "rb"))
			{
				bool result = m_AfxColorLut->LoadFromFile(file);

				fclose(file);
				return result;
			}

			return false;
		}
		finally
		{
			System::Runtime::InteropServices::Marshal::FreeHGlobal(pszFileName);
		}
	}

	bool SaveToFile(System::String^ fileName)
	{
		IntPtr pszFileName = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileName);
		try
		{
			FILE* file = nullptr;

			if (0 == fopen_s(&file, static_cast<const char*>(pszFileName.ToPointer()), "wb"))
			{
				bool result = m_AfxColorLut->SaveToFile(file);

				fclose(file);
				return result;
			}

			return false;
		}
		finally
		{
			System::Runtime::InteropServices::Marshal::FreeHGlobal(pszFileName);
		}
	}

	bool Query(float r, float g, float b, float a,
		[System::Runtime::InteropServices::Out] float% outR,
		[System::Runtime::InteropServices::Out] float% outG,
		[System::Runtime::InteropServices::Out] float% outB,
		[System::Runtime::InteropServices::Out] float% outA)
	{
		float tOutR, tOutG, tOutB, tOutA;

		if (bool result = m_AfxColorLut->Query(r, g, b, a, tOutR, tOutG, tOutB, tOutA))
		{
			outR = tOutR;
			outG = tOutG;
			outB = tOutB;
			outA = tOutA;

			return true;
		}

		return false;
	}

	[returnvalue: System::Runtime::InteropServices::MarshalAs(System::Runtime::InteropServices::UnmanagedType::Bool)]
	delegate bool IteratePutCallBack(float r, float g, float b, float a,
		[System::Runtime::InteropServices::Out] float% outR,
		[System::Runtime::InteropServices::Out] float% outG,
		[System::Runtime::InteropServices::Out] float% outB,
		[System::Runtime::InteropServices::Out] float% outA);

	bool IteratePut(IteratePutCallBack ^ callback)
	{
		GCHandle gch = GCHandle::Alloc(callback);

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		CAfxColorLut::IteratePutCallback_t nativeCallback = static_cast<CAfxColorLut::IteratePutCallback_t>(ip.ToPointer());

		bool result = m_AfxColorLut->IteratePut(nativeCallback);

		gch.Free();

		return result;
	}

private:
	CAfxColorLut * m_AfxColorLut;
};

} // namespace AfxCppCli {

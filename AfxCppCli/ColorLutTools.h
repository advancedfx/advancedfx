#pragma once

#include <shared/AfxColorLut.h>
#include <stdio.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace AfxCppCli {

ref class ColorLutTools
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

	bool LoadFromFile(const char* fileName)
	{
		FILE* file = nullptr;
		if (0 == fopen_s(&file, fileName, "rb"))
		{
			bool result = m_AfxColorLut->LoadFromFile(file);

			fclose(file);
			return result;
		}

		return false;
	}

	bool SaveToFile(const char* fileName)
	{
		FILE* file = nullptr;
		if (0 == fopen_s(&file, fileName, "wb"))
		{
			bool result = m_AfxColorLut->SaveToFile(file);

			fclose(file);
			return result;
		}

		return false;
	}

	bool Query(System::Drawing::Color^ color, System::Drawing::Color^ &outColor)
	{
		CAfxColorLut::CRgba rgba(
			color->R / 255.0f,
			color->G / 255.0f,
			color->B / 255.0f,
			color->A / 255.0f
		);

		CAfxColorLut::CRgba outRgba(rgba);

		if (bool result = m_AfxColorLut->Query(rgba, &outRgba))
		{
			outColor = System::Drawing::Color::FromArgb(
				(int)System::Math::Max(0.0f, System::Math::Min(outRgba.A * 255.0f + 0.5f, 255.0f)),
				(int)System::Math::Max(0.0f, System::Math::Min(outRgba.R * 255.0f + 0.5f, 255.0f)),
				(int)System::Math::Max(0.0f, System::Math::Min(outRgba.G * 255.0f + 0.5f, 255.0f)),
				(int)System::Math::Max(0.0f, System::Math::Min(outRgba.B * 255.0f + 0.5f, 255.0f)));

			return true;
		}

		return false;
	}

	delegate bool IteratePutCallBack(float r, float g, float b, float a,
		[System::Runtime::InteropServices::OutAttribute] Single% outR,
		[System::Runtime::InteropServices::OutAttribute] Single% outG,
		[System::Runtime::InteropServices::OutAttribute] Single% outB,
		[System::Runtime::InteropServices::OutAttribute] Single% outA);

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

#pragma once

#include <shared/AfxColorLut.h>
#include <stdio.h>

namespace AfxCppCli {

ref class ColorLutTools
{
public:
	ColorLutTools()
	{
		m_AfxColorLut = new CAfxColorLut();
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

	void Put(System::Drawing::Color^key, System::Drawing::Color^value)
	{
		CAfxColorLut::CRgba rgbaKey(
			key->R / 255.0f,
			key->G / 255.0f,
			key->B / 255.0f,
			key->A / 255.0f
		);
		CAfxColorLut::CRgba rgbaValue(
			value->R / 255.0f,
			value->G / 255.0f,
			value->B / 255.0f,
			value->A / 255.0f
		);

		//m_AfxColorLut->Put(rgbaKey, rgbaValue);
	}

	~ColorLutTools()
	{
		delete m_AfxColorLut;
	}

private:
	CAfxColorLut * m_AfxColorLut;
};

} // namespace AfxCppCli {

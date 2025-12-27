#pragma once

namespace advancedfx {

enum class ImageFormat : unsigned int {
	Unknown = 0,
	BGR = 1,
	BGRA = 2,
	A = 3,
	ZFloat = 4,
	RGBA = 5
};

enum class ImageOrigin : unsigned int {
	Unknown = 0,
	TopLeft = 1,
	BottomLeft = 2
};

class CImageFormat
{
public:
	ImageFormat Format;
	int Width;
	int Height;
	size_t Pitch;
	size_t Bytes;
	ImageOrigin Origin;

	CImageFormat()
		: Format(ImageFormat::Unknown)
		, Width(0)
		, Height(0)
		, Pitch(0)
		, Origin(ImageOrigin::Unknown)
	{
		Calc();
	}

	CImageFormat(ImageFormat format, int width, int height)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(0)
		, Origin(ImageOrigin::TopLeft)
	{
		Calc();
	}


	CImageFormat(ImageFormat format, int width, int height, size_t pitch)
		: Format(format)
		, Width(width)
		, Height(height)
		, Pitch(pitch)
		, Origin(ImageOrigin::TopLeft)
	{
		Calc();
	}

	bool operator==(const CImageFormat & other) const
	{
		return Format == other.Format
			&& Width == other.Width
			&& Height == other.Height
			&& Pitch == other.Pitch
			&& Bytes == other.Bytes
			&& Origin == other.Origin;
	}

	bool operator!=(const CImageFormat & other) const {
		return !this->operator==(other);
	}

	void SetOrigin(ImageOrigin origin) {
		Origin = origin;
	}

	size_t GetLineStride() const {
		return Pitch;
	}

	size_t GetPixelStride() const {
		switch (Format)
		{
		case ImageFormat::BGR:
			return 3 * sizeof(unsigned char);
		case ImageFormat::BGRA:
			return 4 * sizeof(unsigned char);
		case ImageFormat::A:
			return 1 * sizeof(unsigned char);
		case ImageFormat::ZFloat:
			return 1 * sizeof(float);
		}

		return 0;	
	}

private:
	void Calc()
	{
		if (Pitch == 0)
		{
			Pitch = Width * GetPixelStride();
		}

		Bytes = Height * Pitch;
	}
};

} // namespace advancedfx {

#pragma once

#include "TRefCounted.h"
#include "RefCounted.h"
#include "RefCountedThreadSafe.h"
#include "TImageBuffer.h"
#include "EasySampler.h"

#include <mutex>
#include <string>
#include <list>
#include <map>
#include <Windows.h>

namespace advancedfx {


class COutStream
{
public:
	enum Type {
		Type_Audio,
		Type_Video,
		Type_AudioVideo
	};

	Type GetMediaType() const
	{
		return m_Type;
	}

protected:
	COutStream(Type mediaType)
		: m_Type(mediaType)
	{
	}

private:
	Type m_Type;
};

class COutAudioStream : public COutStream
{
public:
	unsigned int GetChannels() const
	{
		return m_Channles;
	}

	virtual bool SupplyAudioData(unsigned int channels, unsigned int samples, const float* data) = 0;

protected:
	COutAudioStream(unsigned int channels)
		: COutStream(Type_Audio)
		, m_Channles(channels)
	{

	}

	unsigned int m_Channles;
};

class COutVideoStreamImpl : public COutStream
{
protected:
	COutVideoStreamImpl(const CImageFormat& imageFormat)
		: COutStream(Type_Audio)
		, m_ImageFormat(imageFormat)
	{

	}

	const CImageFormat m_ImageFormat;
};

template<bool bThreadSafe> class TIOutVideoStream
{
public:
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<bThreadSafe> * pImageBuffer) = 0;
};

class COutImageStreamImpl
: public COutVideoStreamImpl
{
protected:
	COutImageStreamImpl(const CImageFormat& imageFormat, const std::wstring& path, bool ifZip, bool ifBmpNotTga)
		: COutVideoStreamImpl(imageFormat)
		, m_Path(path)
		, m_IfZip(ifZip)
		, m_IfBmpNotTga(ifBmpNotTga)
	{

	}

	bool WriteBuffer(const unsigned char* pBuffer);

private:
	std::wstring m_Path;
	bool m_IfZip;
	bool m_IfBmpNotTga;

	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath = false;

	size_t m_FrameNumber = 0;

	bool CreateCapturePath(const char* fileExtension, std::wstring& outPath);
};

template<bool bThreadSafe> class COutImageStream
: public COutImageStreamImpl
, public TRefCounted<bThreadSafe>
, public TIOutVideoStream<bThreadSafe>
{
public:
	COutImageStream(const CImageFormat& imageFormat, const std::wstring& path, bool ifZip, bool ifBmpNotTga)
	: COutImageStreamImpl(imageFormat, path, ifZip, ifBmpNotTga) {
	}

	virtual void AddRef() override {
		TRefCounted<bThreadSafe>::AddRef();
	}

	virtual void Release() override {
		TRefCounted<bThreadSafe>::Release();
	}

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<bThreadSafe> * pImageBuffer) override {
		if (nullptr == pImageBuffer || *pImageBuffer->GetImageBufferFormat() != m_ImageFormat)
			return false;
		const unsigned char* pBuffer = static_cast<const unsigned char*>(pImageBuffer->GetImageBufferData());
		return WriteBuffer(pBuffer);
	}
};

class COutFFMPEGVideoStreamImpl : public COutVideoStreamImpl
{
protected:
	COutFFMPEGVideoStreamImpl(const CImageFormat& imageFormat, const std::wstring& path, const std::wstring& ffmpegOptions, float frameRate);

	virtual ~COutFFMPEGVideoStreamImpl();

	bool WriteBuffer(const unsigned char* pBuffer);

private:
	PROCESS_INFORMATION m_ProcessInfo;
	bool m_TriedCreatePath = false;
	bool m_SucceededCreatePath = false;
	BOOL m_Okay = FALSE;
	HANDLE m_hChildStd_IN_Rd = NULL;
	HANDLE m_hChildStd_IN_Wr = NULL;
	HANDLE m_hChildStd_OUT_Rd = NULL;
	HANDLE m_hChildStd_OUT_Wr = NULL;
	HANDLE m_hChildStd_ERR_Rd = NULL;
	HANDLE m_hChildStd_ERR_Wr = NULL;
	OVERLAPPED m_OverlappedStdin = {};
	//OVERLAPPED m_OverlappedStdout = {};
	//OVERLAPPED m_OverlappedStderr = {};

	void Close();

	bool HandleOutAndErr(DWORD processWaitTimeOut = 0);
};

template<bool bThreadSafe> class COutFFMPEGVideoStream
: public COutFFMPEGVideoStreamImpl
, public TRefCounted<bThreadSafe>
, public TIOutVideoStream<bThreadSafe>
{
public:
	COutFFMPEGVideoStream(const CImageFormat& imageFormat, const std::wstring& path, const std::wstring& ffmpegOptions, float frameRate)
	: COutFFMPEGVideoStreamImpl(imageFormat, path, ffmpegOptions, frameRate)
	{

	}

	virtual void AddRef() override {
		TRefCounted<bThreadSafe>::AddRef();
	}

	virtual void Release() override {
		TRefCounted<bThreadSafe>::Release();
	}	

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<bThreadSafe> * pImageBuffer) override {
		if (nullptr == pImageBuffer || *pImageBuffer->GetImageBufferFormat() != m_ImageFormat)
			return false;
		const unsigned char* pBuffer = static_cast<const unsigned char*>(pImageBuffer->GetImageBufferData());
		return WriteBuffer(pBuffer);
	}
};


// TODO: The sampler could work in parallel on the image.
// TODO:
// - optimize shutter to allow skipping (capturing of) frames.
// - think about error propagation, though not entirely applicable.
// - RGBA smampling might not be accurate, since it doesn't take alpha into account?
template<bool bThreadSafe> class COutSamplingStream
: public COutVideoStreamImpl
, public TRefCounted<bThreadSafe>
, public TIOutVideoStream<bThreadSafe>
, public IFramePrinter<bThreadSafe>
{
public:
	COutSamplingStream(const CImageFormat& imageFormat, TIOutVideoStream<true>* outVideoStream, float frameRate, EasySamplerSettings::Method method, double frameDuration, double exposure, float frameStrength, TGrowingBufferPool<bThreadSafe>* imageBufferPool)
	: COutVideoStreamImpl(imageFormat)
	, m_OutVideoStream(outVideoStream)
	, m_Time(0.0)
	, m_InputFrameDuration(frameRate ? 1.0 / frameRate : 0.0)
	, m_ImageBufferPool(imageBufferPool)
	{
		if (m_OutVideoStream) m_OutVideoStream->AddRef();

		switch (imageFormat.Format)
		{
		case ImageFormat::BGR:
		case ImageFormat::BGRA:
		case ImageFormat::A:
		case ImageFormat::RGBA:
			m_EasySampler.Byte = new EasyByteSampler<bThreadSafe>(EasySamplerSettings(
				imageFormat,
				method,
				frameDuration,
				m_Time,
				exposure,
				frameStrength
			), this, m_ImageBufferPool);
			break;
		case ImageFormat::ZFloat:
			m_EasySampler.Float = new EasyFloatSampler<bThreadSafe>(EasySamplerSettings(
				imageFormat,
				method,
				frameDuration,
				m_Time,
				exposure,
				frameStrength
			), this, m_ImageBufferPool);
			break;
		default:
			advancedfx::Warning("AFXERROR: COutSamplingStream::COutSamplingStream: Unsupported image format.");
		}
	}

	virtual void AddRef() override {
		TRefCounted<bThreadSafe>::AddRef();
	}

	virtual void Release() override {
		TRefCounted<bThreadSafe>::Release();
	}	

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<bThreadSafe> * pImageBuffer) override
	{
		if (nullptr == m_OutVideoStream) return false;

		if (nullptr == pImageBuffer || *pImageBuffer->GetImageBufferFormat() != m_ImageFormat)
				return false;
		const unsigned char* pBuffer = static_cast<const unsigned char*>(pImageBuffer->GetImageBufferData());

		switch (m_ImageFormat.Format)
		{
		case ImageFormat::BGR:
		case ImageFormat::BGRA:
		case ImageFormat::A:
		case ImageFormat::RGBA:
			m_EasySampler.Byte->Sample(pImageBuffer, m_Time);
			break;
		case ImageFormat::ZFloat:
			m_EasySampler.Float->Sample(pImageBuffer, m_Time);
			break;
		};

		m_Time += m_InputFrameDuration;

		return true;
	}

	// Implements IFramePrinter<bThreadSafe>:
	virtual void PrintSampledFrame(advancedfx::TImageBuffer<bThreadSafe> * pImageBuffer) override{
		m_OutVideoStream->SupplyImageBuffer(this, pImageBuffer);
	}	

protected:
	virtual ~COutSamplingStream()
	{
		switch (m_ImageFormat.Format)
		{
		case ImageFormat::BGR:
		case ImageFormat::BGRA:
		case ImageFormat::A:
		case ImageFormat::RGBA:
			delete m_EasySampler.Byte;
			break;
		case ImageFormat::ZFloat:
			delete m_EasySampler.Float;
			break;
		};

		if (m_OutVideoStream) m_OutVideoStream->Release();
	}

private:
	union {
		EasyByteSampler<bThreadSafe>* Byte;
		EasyFloatSampler<bThreadSafe>* Float;
	} m_EasySampler;
	TIOutVideoStream<true>* m_OutVideoStream;
	double m_Time;
	double m_InputFrameDuration;
	TGrowingBufferPool<bThreadSafe>* m_ImageBufferPool;
};


// TODO: The out streams could work in parallel on the image.
template<bool bThreadSafe> class COutMultiVideoStream
: public COutVideoStreamImpl
, public TRefCounted<bThreadSafe>
, public TIOutVideoStream<bThreadSafe>
{
public:
	COutMultiVideoStream(const CImageFormat& imageFormat, std::list<TIOutVideoStream<bThreadSafe>*>&& outStreams)
		: COutVideoStreamImpl(imageFormat)
		, m_OutStreams(outStreams)
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (TIOutVideoStream<bThreadSafe>* stream = *it) stream->AddRef();
		}
	}

	virtual void AddRef() override {
		TRefCounted<bThreadSafe>::AddRef();
	}

	virtual void Release() override {
		TRefCounted<bThreadSafe>::Release();
	}

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<bThreadSafe> * pImageBuffer) override
	{
		bool okay = true;

		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (TIOutVideoStream<bThreadSafe>* stream = *it)
			{
				if (!stream->SupplyImageBuffer(this, pImageBuffer)) okay = false;
			}
		}

		return okay;
	}

protected:
	virtual ~COutMultiVideoStream() override
	{
		for (auto it = m_OutStreams.begin(); it != m_OutStreams.end(); ++it)
		{
			if (TIOutVideoStream<bThreadSafe>* stream = *it) stream->Release();
		}
	}

private:
	std::list<TIOutVideoStream<bThreadSafe>*> m_OutStreams;
};




template<bool bThreadSafe> class COutInterleaveVideoStream;

template<> class COutInterleaveVideoStream<true>
: public COutVideoStreamImpl
, public TRefCounted<true>
, public TIOutVideoStream<true>
{
public:
	COutInterleaveVideoStream(const CImageFormat& imageFormat, TIOutVideoStream<true>* outStream, int inputCount)
		: COutVideoStreamImpl(imageFormat)
		, m_OutStream(outStream)
		, m_InputCount(inputCount)
	{
		if(m_OutStream) m_OutStream->AddRef();
	}

	virtual void AddRef() override {
		TRefCounted<true>::AddRef();
	}

	virtual void Release() override {
		TRefCounted<true>::Release();
	}

	virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<true> * pImageBuffer) override
	{
		return InputSupplyImageBuffer(this, pImageBuffer, 0);
	}

	class TIOutVideoStream<true> * AddInput(size_t index) {
		auto result = new CInterleaveInput(this, index);
		result->AddRef();
		return result;
	}

protected:
	virtual ~COutInterleaveVideoStream() override
	{
		for(auto it = m_Buffers.begin(); it != m_Buffers.end(); it++) {
			for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				(*it2)->Release();
			}
		}

		if(m_OutStream) {
			m_OutStream->Release();
			m_OutStream = nullptr;
		}
	}

private:
	class CInterleaveInput
	: public TRefCounted<true>
	, public TIOutVideoStream<true>
	{
	public:
		CInterleaveInput(COutInterleaveVideoStream<true> * mainInput, size_t index)
		: TRefCounted<true>()
		, m_MainInput(mainInput)
		, m_Index(index)
		{
			m_MainInput->AddRef();
		}

		virtual void AddRef() override {
			TRefCounted<true>::AddRef();
		}

		virtual void Release() override {
			TRefCounted<true>::Release();
		}

		virtual bool SupplyImageBuffer(void * pSourceId, TIImageBuffer<true> * pImageBuffer) {
			return m_MainInput->InputSupplyImageBuffer(this, pImageBuffer, m_Index);
		}

	protected:
		virtual ~CInterleaveInput() override {
			m_MainInput->Release();
		}

	private:
		COutInterleaveVideoStream * m_MainInput;
		size_t m_Index;
	};
	
	size_t m_InputCount;
	TIOutVideoStream<true>* m_OutStream;
	std::mutex m_BuffersMutex;
	std::map<size_t,std::list<TIImageBuffer<true> *>> m_Buffers;

	bool InputSupplyImageBuffer(void * pSourceId, TIImageBuffer<true> * pImageBuffer, size_t index) {

		bool result = true;

		std::list<TIImageBuffer<true> *> buffers;

		if(pImageBuffer) pImageBuffer->AddRef();

		{
			std::unique_lock<std::mutex> lock(m_BuffersMutex);
			m_Buffers[index].emplace_back(pImageBuffer);

			while(m_Buffers.size() >= m_InputCount) {
				auto it = m_Buffers.begin();
				while(it != m_Buffers.end()) {
					auto & list = it->second;
					auto it_list = list.begin();
					buffers.emplace_back(*it_list);
					list.erase(it_list);
					if(list.empty()) {
						it = m_Buffers.erase(it);
					} else {
						it++;
					}
				}
			}
		}

		if(0 < buffers.size()) {
			for(auto it = buffers.begin(); it != buffers.end(); it++) {
				if(m_OutStream) {
					if(!m_OutStream->SupplyImageBuffer(this, *it)) result = false;
				}
				(*it)->Release();
			}
		}

		return result;
	}
};


} // namespace advancedfx {

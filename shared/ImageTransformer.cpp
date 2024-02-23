#include "stdafx.h"
#include "ImageTransformer.h"
#include "RefCountedThreadSafe.h"
#include "AfxConsole.h"

namespace advancedfx {
namespace ImageTransformer {

	class CAfxImageBufferCapture
		: public advancedfx::CRefCountedThreadSafe
		, public ICapture
	{
	public:
		static class CAfxImageBufferCapture* Create(class IImageBufferPool * imageBufferPool, const class advancedfx::CImageFormat& format) {
			class CAfxImageBufferCapture* result = new CAfxImageBufferCapture(imageBufferPool);
			result->AddRef();
			if (!result->AutoRealloc(format)) {
				result->Release();
				advancedfx::Warning("CAfxImageBufferCapture::Create: Failed to reallocate buffer.\n");
				return nullptr;
			}
			return result;
		}

		virtual void AddRef() override {
			advancedfx::CRefCountedThreadSafe::AddRef();
		}

		virtual void Release() override {
			advancedfx::CRefCountedThreadSafe::Release();
		}

		virtual const advancedfx::IImageBuffer* GetBuffer() const {
			return m_ImageBuffer;
		}

		void * GetImageBufferDataRw() const {
			return m_ImageBuffer->Buffer;
		}

	protected:
		CAfxImageBufferCapture(class IImageBufferPool * imageBufferPool)
			: advancedfx::CRefCountedThreadSafe()
            , m_ImageBufferPool(imageBufferPool)
		{
			m_ImageBuffer = m_ImageBufferPool->AquireBuffer();
		}

		virtual ~CAfxImageBufferCapture() {
			m_ImageBufferPool->ReleaseBuffer(m_ImageBuffer);
		}

	private:
    	advancedfx::CImageBuffer* m_ImageBuffer;
        class IImageBufferPool * m_ImageBufferPool;
	
		bool AutoRealloc(const class advancedfx::CImageFormat& format) {
			return m_ImageBuffer->AutoRealloc(format);
		}
	};

	class CTranformTask
		: public advancedfx::CThreadPool::CTask
	{
	public:
		CTranformTask(std::atomic_int& task_counter)
			: task_counter(task_counter)			
		{
			task_counter++;
		}

		virtual ~CTranformTask() {
			task_counter--;
		}

	protected:

	private:
		std::atomic_int& task_counter;
	};

	class ITransform {
	public:
		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) = 0;
		virtual size_t GetTaskSize() = 0;
		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) = 0;
	};
	class CTransformAColorBRedAsAlpha
		: public ITransform {
	public:
		CTransformAColorBRedAsAlpha(class ICapture* aColor, class ICapture* bRedAsAlpha)
			: m_CaptureA(aColor)
			, m_CaptureB(bRedAsAlpha)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_CaptureA && nullptr != m_CaptureB) {

				if (const class advancedfx::IImageBuffer* pBufferA = m_CaptureA->GetBuffer()) {
					if (const unsigned char* pDataA = static_cast<const unsigned char*>(pBufferA->GetImageBufferData())) {
						m_pInDataA = pDataA;
						if (const class advancedfx::CImageFormat* pFormatA = pBufferA->GetImageBufferFormat()) {
							m_InFormatA = *pFormatA;
							if (m_InFormatA.Format == advancedfx::ImageFormat::BGRA || m_InFormatA.Format == advancedfx::ImageFormat::BGR) {

								if (const class advancedfx::IImageBuffer* pBufferB = m_CaptureB->GetBuffer()) {
									if (const unsigned char* pDataB = static_cast<const unsigned char*>(pBufferB->GetImageBufferData())) {
										m_pInDataB = pDataB;
										if (const class advancedfx::CImageFormat* pFormatB = pBufferB->GetImageBufferFormat()) {
											m_InFormatB = *pFormatB;
											if (
												(m_InFormatB.Format == advancedfx::ImageFormat::BGRA || m_InFormatB.Format == advancedfx::ImageFormat::BGR)
												&& m_InFormatB.Width == m_InFormatA.Width
												&& m_InFormatB.Height == m_InFormatA.Height
												&& m_InFormatB.Origin == m_InFormatA.Origin)
											{
												m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatA.Width, m_InFormatA.Height);
												m_OutFormat.SetOrigin(m_InFormatA.Origin);
												class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool,m_OutFormat);
												if (pOutCapture) {
													m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
												}
												return pOutCapture;
											}
										}
									}
								}

							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_OutFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter,
				m_pInDataA + taskIndex * m_InFormatA.Pitch, m_InFormatA.Pitch, m_InFormatA.GetPixelStride(),
				m_pInDataB + taskIndex * m_InFormatB.Pitch, m_InFormatB.Pitch, m_InFormatB.GetPixelStride(),
				m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pDataA, size_t pitchA, size_t pixelPichtA, const unsigned char* pDataB, size_t pitchB, size_t pixelPichtB, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pDataA(pDataA)
				, pitchA(pitchA)
				, pixelPichtA(pixelPichtA)
				, pDataB(pDataB)
				, pitchB(pitchB)
				, pixelPichtB(pixelPichtB)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 4 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char* pInA = (const unsigned char*)(pDataA + y * pitchA + x * pixelPichtA);
						const unsigned char* pInB = (const unsigned char*)(pDataB + y * pitchB + x * pixelPichtB);
						unsigned char* pOut = (unsigned char*)(pOutData + y * targetPitch + x * 4 * sizeof(unsigned char));

						pOut[0] = pInA[0];
						pOut[1] = pInA[1];
						pOut[2] = pInA[2];
						pOut[3] = pInB[0];

					}
				}
			}

		private:
			const unsigned char* pDataA;
			size_t pitchA;
			size_t pixelPichtA;
			const unsigned char* pDataB;
			size_t pitchB;
			size_t pixelPichtB;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_CaptureA;
		class ICapture* m_CaptureB;
		advancedfx::CImageFormat m_InFormatA;
		advancedfx::CImageFormat m_InFormatB;
		const unsigned char* m_pInDataA;
		const unsigned char* m_pInDataB;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformMatte
		: public ITransform {
	public:
		CTransformMatte(class ICapture* captureEntBlack, class ICapture* captureEntWhite)
			: m_CaptureEntBlack(captureEntBlack)
			, m_CaptureEntWhite(captureEntWhite)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_CaptureEntBlack && nullptr != m_CaptureEntWhite) {

				if (const class advancedfx::IImageBuffer* pBufferEntBlack = m_CaptureEntBlack->GetBuffer()) {
					if (const unsigned char* pDataEntBlack = static_cast<const unsigned char*>(pBufferEntBlack->GetImageBufferData())) {
						m_pInDataEntBlack = pDataEntBlack;
						if (const class advancedfx::CImageFormat* pFormatEntBlack = pBufferEntBlack->GetImageBufferFormat()) {
							m_InFormatEntBlack = *pFormatEntBlack;
							if (m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGR) {

								if (const class advancedfx::IImageBuffer* pBufferEntWhite = m_CaptureEntWhite->GetBuffer()) {
									if (const unsigned char* pDataEntWhite = static_cast<const unsigned char*>(pBufferEntWhite->GetImageBufferData())) {
										m_pInDataEntWhite = pDataEntWhite;
										if (const class advancedfx::CImageFormat* pFormatEntWhite = pBufferEntWhite->GetImageBufferFormat()) {
											m_InFormatEntWhite = *pFormatEntWhite;
											if (
												(m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGR)
												&& m_InFormatEntWhite.Width == m_InFormatEntBlack.Width
												&& m_InFormatEntWhite.Height == m_InFormatEntBlack.Height
												&& m_InFormatEntWhite.Origin == m_InFormatEntBlack.Origin)
											{
												m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatEntBlack.Width, m_InFormatEntBlack.Height);
												m_OutFormat.SetOrigin(m_InFormatEntBlack.Origin);
												class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
												if (pOutCapture) {
													m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
												}
												return pOutCapture;
											}
										}
									}
								}

							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_OutFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter,
				m_pInDataEntBlack + taskIndex * m_InFormatEntBlack.Pitch, m_InFormatEntBlack.Pitch, m_InFormatEntBlack.GetPixelStride(),
				m_pInDataEntWhite + taskIndex * m_InFormatEntWhite.Pitch, m_InFormatEntWhite.Pitch, m_InFormatEntWhite.GetPixelStride(),
				m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pDataEntBlack, size_t pitchEntBlack, size_t pixelPichtEntBlack, const unsigned char* pDataEntWhite, size_t pitchEntWhite, size_t pixelPichtEntWhite, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pDataEntBlack(pDataEntBlack)
				, pitchEntBlack(pitchEntBlack)
				, pixelPichtEntBlack(pixelPichtEntBlack)
				, pDataEntWhite(pDataEntWhite)
				, pitchEntWhite(pitchEntWhite)
				, pixelPichtEntWhite(pixelPichtEntWhite)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 4 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char* pInEntBlack = (const unsigned char*)(pDataEntBlack + y * pitchEntBlack + x * pixelPichtEntBlack);
						const unsigned char* pInEntWhite = (const unsigned char*)(pDataEntWhite + y * pitchEntWhite + x * pixelPichtEntWhite);
						unsigned char* pOut = (unsigned char*)(pOutData + y * targetPitch + x * 4 * sizeof(unsigned char));

						unsigned char entBlack_b = pInEntBlack[0];
						unsigned char entBlack_g = pInEntBlack[1];
						unsigned char entBlack_r = pInEntBlack[2];

						unsigned char entWhite_b = pInEntWhite[0];
						unsigned char entWhite_g = pInEntWhite[1];
						unsigned char entWhite_r = pInEntWhite[2];

						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 0] = y < 1 * height / 3 ? entBlack_b : (y < 2 * height / 3 ? entWhite_b : (unsigned char)(((int)entBlack_b + (int)entWhite_b)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 1] = y < 1 * height / 3 ? entBlack_g : (y < 2 * height / 3 ? entWhite_g : (unsigned char)(((int)entBlack_g + (int)entWhite_g)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 2] = y < 1 * height / 3 ? entBlack_r : (y < 2 * height / 3 ? entWhite_r : (unsigned char)(((int)entBlack_r + (int)entWhite_r)/2));
						//((unsigned char *)pBufferEntBlack)[y*newImagePitchA + x * 4 + 3] = y < 1 * height / 3 ? 255 : (y < 2 * height / 3 ? 255 : (unsigned char)min(max((255l - (int)entWhite_b + (int)entBlack_b + 255l - (int)entWhite_g + (int)entBlack_g + 255l - (int)entWhite_r + (int)entBlack_r) / 3l, 0), 255));
						pOut[0] = (unsigned char)(((int)entBlack_b + (int)entWhite_b) / 2);
						pOut[1] = (unsigned char)(((int)entBlack_g + (int)entWhite_g) / 2);
						pOut[2] = (unsigned char)(((int)entBlack_r + (int)entWhite_r) / 2);
						pOut[3] = (unsigned char)std::min(std::max((255l - (int)entWhite_b + (int)entBlack_b + 255l - (int)entWhite_g + (int)entBlack_g + 255l - (int)entWhite_r + (int)entBlack_r) / 3l, 0l), 255l);

					}
				}
			}

		private:
			const unsigned char* pDataEntBlack;
			size_t pitchEntBlack;
			size_t pixelPichtEntBlack;
			const unsigned char* pDataEntWhite;
			size_t pitchEntWhite;
			size_t pixelPichtEntWhite;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_CaptureEntBlack;
		class ICapture* m_CaptureEntWhite;
		advancedfx::CImageFormat m_InFormatEntBlack;
		advancedfx::CImageFormat m_InFormatEntWhite;
		const unsigned char* m_pInDataEntBlack;
		const unsigned char* m_pInDataEntWhite;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformStripAlpha
		: public ITransform {
	public:
		CTransformStripAlpha(class ICapture* capture)
			: m_Capture(capture)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGR, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 3 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char * pIn = (const unsigned char *)(pData + y * pitch + x * 4 * sizeof(unsigned char));
						unsigned char * pOut = (unsigned char*)(pOutData + y * targetPitch + x * 3 * sizeof(unsigned char));
	
						pOut[0] = pIn[0];
						pOut[1] = pIn[1];
						pOut[2] = pIn[2];
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformRgbaToBgr
		: public ITransform {
	public:
		CTransformRgbaToBgr(class ICapture* capture)
			: m_Capture(capture)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::RGBA) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGR, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, unsigned char* pOutData, size_t width, size_t height)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * 3 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char * pIn = (const unsigned char *)(pData + y * pitch + x * 4 * sizeof(unsigned char));
						unsigned char * pOut = (unsigned char*)(pOutData + y * targetPitch + x * 3 * sizeof(unsigned char));
	
						pOut[0] = pIn[2];
						pOut[1] = pIn[1];
						pOut[2] = pIn[0];
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformDepthF 
		: public ITransform {
	public:
		CTransformDepthF(class ICapture* capture, float depthScale, float depthOfs)
			: m_Capture(capture)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::ZFloat) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual CAfxImageBufferCapture* CreateCapture(class IImageBufferPool * imageBufferPool) {
			class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
			if(pOutCapture) {
				m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
			}
			return pOutCapture;							
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize, m_DepthScale, m_DepthOfs);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, unsigned char* pOutData, size_t width, size_t height, float depthScale, float depthOfs)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
				, depthScale(depthScale)
				, depthOfs(depthOfs)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * sizeof(float);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						float depth = *(const float*)(pData + y * pitch + x * sizeof(float));

						depth *= depthScale;
						depth += depthOfs;

						*(float*)(pOutData + y * targetPitch + x * sizeof(float))
							= depth;
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
			float depthScale;
			float depthOfs;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
		float m_DepthScale;
		float m_DepthOfs;
	};

	class CTransformDepth24
		: public ITransform {
	public:
		CTransformDepth24(class ICapture* capture, float depthScale, float depthOfs)
			: m_Capture(capture)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual CAfxImageBufferCapture* CreateOutput(class IImageBufferPool * imageBufferPool) {
			if (nullptr != m_Capture) {
				if (const class advancedfx::IImageBuffer* pBuffer = m_Capture->GetBuffer()) {
					if (const unsigned char* pData = static_cast<const unsigned char*>(pBuffer->GetImageBufferData())) {
						if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
							m_InFormat = *pFormat;
							if (m_InFormat.Format == advancedfx::ImageFormat::BGR || m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
								m_pInData = pData;
								m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
								m_OutFormat.SetOrigin(m_InFormat.Origin);
								class CAfxImageBufferCapture* pOutCapture = CAfxImageBufferCapture::Create(imageBufferPool, m_OutFormat);
								if (pOutCapture) {
									m_pOutData = static_cast<unsigned char*>(pOutCapture->GetImageBufferDataRw());
								}
								return pOutCapture;
							}
						}
					}
				}
			}

			return nullptr;
		}

		virtual size_t GetTaskSize() {
			return (size_t)std::abs(m_InFormat.Height);
		}

		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) {
			return new CMyTransformTask(task_counter, m_pInData + taskIndex * m_InFormat.Pitch, m_InFormat.Pitch, m_InFormat.GetPixelStride(), m_pOutData + taskIndex * m_OutFormat.Pitch, m_OutFormat.Width, taskSize, m_DepthScale, m_DepthOfs);
		}

	private:
		class CMyTransformTask : public CTranformTask {
		public:
			CMyTransformTask(std::atomic_int& task_counter, const unsigned char* pData, size_t pitch, size_t pixelPitch, unsigned char* pOutData, size_t width, size_t height, float depthScale, float depthOfs)
				: CTranformTask(task_counter)
				, pData(pData)
				, pitch(pitch)
				, pixelPitch(pixelPitch)
				, pOutData(pOutData)
				, width(width)
				, height(height)
				, depthScale(depthScale)
				, depthOfs(depthOfs)
			{
			}

			virtual void Execute() {
				size_t targetPitch = width * sizeof(float);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char b = pData[y * pitch + x * pixelPitch + 0];
						const unsigned char g = pData[y * pitch + x * pixelPitch + 1];
						const unsigned char r = pData[y * pitch + x * pixelPitch + 2];

						float depth;

						depth = (1.0f / 16777215.0f) * r + (256.0f / 16777215.0f) * g + (65536.0f / 16777215.0f) * b;

						depth *= depthScale;
						depth += depthOfs;

						*(float*)(pOutData + y * targetPitch + x * sizeof(float)) = depth;
					}
				}
			}

		private:
			const unsigned char* pData;
			size_t pitch;
			size_t pixelPitch;
			unsigned char* pOutData;
			size_t width;
			size_t height;
			float depthScale;
			float depthOfs;
		};

		class ICapture* m_Capture;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
		float m_DepthScale;
		float m_DepthOfs;
	};

class ICapture* Transform(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ITransform* transform) {
    if (class CAfxImageBufferCapture* pOutCapture = transform->CreateOutput(imageBufferPool)) {
        size_t outTaskSize = transform->GetTaskSize();
        size_t thread_count = std::min(threadPool->GetThreadCount() + 1, outTaskSize);
        size_t lines_per_task = outTaskSize / thread_count;
        size_t lines_per_task_remainder = outTaskSize % thread_count;
        std::atomic_int task_counter(0);
        size_t line = 0;
        if (1 < thread_count) {
            std::vector<advancedfx::CThreadPool::CTask*> tasks(thread_count - 1);
            for (size_t i = 0; i < tasks.size(); i++) {
                size_t cur_task_lines = lines_per_task;
                if (0 < lines_per_task_remainder) {
                    cur_task_lines += 1;
                    lines_per_task_remainder--;
                }
                tasks[i] = transform->CreateTask(task_counter, line, cur_task_lines);
                line += cur_task_lines;
            }
            threadPool->QueueTasks(tasks);
        }
        {
            advancedfx::CThreadPool::CTask* lastTask = transform->CreateTask(task_counter, line, lines_per_task);
            lastTask->Execute();
            delete lastTask;
        }
        while (0 < task_counter) {}

        return pOutCapture;
    }

    return nullptr;
}

/*
static class ICapture* DummyCapture() {
    advancedfx::CImageFormat format(advancedfx::ImageFormat::BGRA, 1280, 720);
    format.SetOrigin(advancedfx::ImageOrigin::TopLeft);
    CAfxImageBufferCapture * result = CAfxImageBufferCapture::Create(format);
    result->AddRef();
    return result;
}
*/

class ICapture* StripAlpha(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture) {

    if (nullptr == capture) return nullptr;

    if (const class advancedfx::IImageBuffer* pBuffer = capture->GetBuffer()) {
        if (const class advancedfx::CImageFormat* pFormat = pBuffer->GetImageBufferFormat()) {
            if (pFormat->Format == advancedfx::ImageFormat::BGR) {
                capture->AddRef();
                return capture;
            }
        }
    }

    CTransformStripAlpha transform(capture);
    return Transform(threadPool, imageBufferPool, &transform);
}

class ICapture* RgbaToBgr(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture) {

    if (nullptr == capture) return nullptr;

    CTransformRgbaToBgr transform(capture);
    return Transform(threadPool, imageBufferPool, &transform);
}

class ICapture* DepthF(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture, float depthScale, float depthOfs) {
    CTransformDepthF transform(capture, depthScale, depthOfs);
    return Transform(threadPool, imageBufferPool, &transform);
}

class ICapture* Depth24(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* capture, float depthScale, float depthOfs) {
    CTransformDepth24 transform(capture, depthScale, depthOfs);
    return Transform(threadPool, imageBufferPool, &transform);
}

class ICapture* Matte(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* captureEntBlack, class ICapture* captureEntWhite) {
    CTransformMatte transform(captureEntBlack, captureEntWhite);
    return Transform(threadPool, imageBufferPool, &transform);
}

class ICapture* AColorBRedAsAlpha(class CThreadPool * threadPool, class IImageBufferPool * imageBufferPool, class ICapture* aColor, class ICapture* bRedAsAlpha) {
    CTransformAColorBRedAsAlpha transform(aColor, bRedAsAlpha);
    return Transform(threadPool, imageBufferPool, &transform);
}


} // namespace ImageTransformer {
} // namespace advancedfx

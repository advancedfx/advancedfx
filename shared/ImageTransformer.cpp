#include "stdafx.h"
#include "ImageTransformer.h"
#include "ImageBufferThreadSafe.h"
#include "RefCountedThreadSafe.h"
#include "AfxConsole.h"

namespace advancedfx {
namespace ImageTransformer {

	static CImageBufferThreadSafe* AquireFormatedImageBuffer(CGrowingBufferPoolThreadSafe * imageBufferPool, const class advancedfx::CImageFormat& format) {
		auto result = new CImageBufferThreadSafe(imageBufferPool);
		result->AddRef();
		if (!result->GrowAlloc(format)) {
			result->Release();
			advancedfx::Warning("AquireFormatedImageBuffer: Failed to GrowAlloc buffer.\n");
			return nullptr;
		}
		return result;
	}

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
		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) = 0;
		virtual size_t GetTaskSize() = 0;
		virtual CTranformTask* CreateTask(std::atomic_int& task_counter, int taskIndex, int taskSize) = 0;
	};
	class CTransformAColorBRedAsAlpha
		: public ITransform {
	public:
		CTransformAColorBRedAsAlpha(IImageBufferThreadSafe* aColor, IImageBufferThreadSafe* bRedAsAlpha)
			: m_BufferA(aColor)
			, m_BufferB(bRedAsAlpha)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_BufferA && nullptr != m_BufferB) {
				if (const unsigned char* pDataA = static_cast<const unsigned char*>(m_BufferA->GetImageBufferData())) {
					m_pInDataA = pDataA;
					if (const class advancedfx::CImageFormat* pFormatA = m_BufferA->GetImageBufferFormat()) {
						m_InFormatA = *pFormatA;
						if (m_InFormatA.Format == advancedfx::ImageFormat::BGRA || m_InFormatA.Format == advancedfx::ImageFormat::BGR) {
							if (const unsigned char* pDataB = static_cast<const unsigned char*>(m_BufferB->GetImageBufferData())) {
								m_pInDataB = pDataB;
								if (const class advancedfx::CImageFormat* pFormatB = m_BufferB->GetImageBufferFormat()) {
									m_InFormatB = *pFormatB;
									if (
										(m_InFormatB.Format == advancedfx::ImageFormat::BGRA || m_InFormatB.Format == advancedfx::ImageFormat::BGR)
										&& m_InFormatB.Width == m_InFormatA.Width
										&& m_InFormatB.Height == m_InFormatA.Height
										&& m_InFormatB.Origin == m_InFormatA.Origin)
									{
										m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatA.Width, m_InFormatA.Height);
										m_OutFormat.SetOrigin(m_InFormatA.Origin);
										CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool,m_OutFormat);
										if (pOutBuffer) {
											m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
										}
										return pOutBuffer;
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

		IImageBufferThreadSafe* m_BufferA;
		IImageBufferThreadSafe* m_BufferB;
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
		CTransformMatte(IImageBufferThreadSafe* bufferEntBlack, IImageBufferThreadSafe* bufferEntWhite)
			: m_BufferEntBlack(bufferEntBlack)
			, m_BufferEntWhite(bufferEntWhite)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_BufferEntBlack && nullptr != m_BufferEntWhite) {
				if (const unsigned char* pDataEntBlack = static_cast<const unsigned char*>(m_BufferEntBlack->GetImageBufferData())) {
					m_pInDataEntBlack = pDataEntBlack;
					if (const class advancedfx::CImageFormat* pFormatEntBlack = m_BufferEntBlack->GetImageBufferFormat()) {
						m_InFormatEntBlack = *pFormatEntBlack;
						if (m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntBlack.Format == advancedfx::ImageFormat::BGR) {
							if (const unsigned char* pDataEntWhite = static_cast<const unsigned char*>(m_BufferEntWhite->GetImageBufferData())) {
								m_pInDataEntWhite = pDataEntWhite;
								if (const class advancedfx::CImageFormat* pFormatEntWhite = m_BufferEntWhite->GetImageBufferFormat()) {
									m_InFormatEntWhite = *pFormatEntWhite;
									if (
										(m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGRA || m_InFormatEntWhite.Format == advancedfx::ImageFormat::BGR)
										&& m_InFormatEntWhite.Width == m_InFormatEntBlack.Width
										&& m_InFormatEntWhite.Height == m_InFormatEntBlack.Height
										&& m_InFormatEntWhite.Origin == m_InFormatEntBlack.Origin)
									{
										m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormatEntBlack.Width, m_InFormatEntBlack.Height);
										m_OutFormat.SetOrigin(m_InFormatEntBlack.Origin);
										CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
										if (pOutBuffer) {
											m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
										}
										return pOutBuffer;
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

		IImageBufferThreadSafe* m_BufferEntBlack;
		IImageBufferThreadSafe* m_BufferEntWhite;
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
		CTransformStripAlpha(IImageBufferThreadSafe* buffer)
			: m_Buffer(buffer)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_Buffer) {
				if (const unsigned char* pData = static_cast<const unsigned char*>(m_Buffer->GetImageBufferData())) {
					if (const class advancedfx::CImageFormat* pFormat = m_Buffer->GetImageBufferFormat()) {
						m_InFormat = *pFormat;
						if (m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
							m_pInData = pData;
							m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGR, m_InFormat.Width, m_InFormat.Height);
							m_OutFormat.SetOrigin(m_InFormat.Origin);
							CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
							if (pOutBuffer) {
								m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
							}
							return pOutBuffer;
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

		IImageBufferThreadSafe* m_Buffer;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformRgbaToBgr
		: public ITransform {
	public:
		CTransformRgbaToBgr(IImageBufferThreadSafe* buffer)
			: m_Buffer(buffer)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_Buffer) {
				if (const unsigned char* pData = static_cast<const unsigned char*>(m_Buffer->GetImageBufferData())) {
					if (const class advancedfx::CImageFormat* pFormat = m_Buffer->GetImageBufferFormat()) {
						m_InFormat = *pFormat;
						if (m_InFormat.Format == advancedfx::ImageFormat::RGBA) {
							m_pInData = pData;
							m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGR, m_InFormat.Width, m_InFormat.Height);
							m_OutFormat.SetOrigin(m_InFormat.Origin);
							CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
							if (pOutBuffer) {
								m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
							}
							return pOutBuffer;
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

		IImageBufferThreadSafe* m_Buffer;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};

	class CTransformRgbaToBgra
		: public ITransform {
	public:
		CTransformRgbaToBgra(IImageBufferThreadSafe* buffer)
			: m_Buffer(buffer)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_Buffer) {
				if (const unsigned char* pData = static_cast<const unsigned char*>(m_Buffer->GetImageBufferData())) {
					if (const class advancedfx::CImageFormat* pFormat = m_Buffer->GetImageBufferFormat()) {
						m_InFormat = *pFormat;
						if (m_InFormat.Format == advancedfx::ImageFormat::RGBA) {
							m_pInData = pData;
							m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::BGRA, m_InFormat.Width, m_InFormat.Height);
							m_OutFormat.SetOrigin(m_InFormat.Origin);
							CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
							if (pOutBuffer) {
								m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
							}
							return pOutBuffer;
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
				size_t targetPitch = width * 4 * sizeof(unsigned char);
				for (size_t y = 0; y < height; ++y)
				{
					for (size_t x = 0; x < width; ++x)
					{
						const unsigned char * pIn = (const unsigned char *)(pData + y * pitch + x * 4 * sizeof(unsigned char));
						unsigned char * pOut = (unsigned char*)(pOutData + y * targetPitch + x * 4 * sizeof(unsigned char));
	
						pOut[0] = pIn[2];
						pOut[1] = pIn[1];
						pOut[2] = pIn[0];
						pOut[3] = pIn[3];
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

		IImageBufferThreadSafe* m_Buffer;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
	};	

	class CTransformDepthF 
		: public ITransform {
	public:
		CTransformDepthF(IImageBufferThreadSafe* buffer, float depthScale, float depthOfs)
			: m_Buffer(buffer)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_Buffer) {
				if (const unsigned char* pData = static_cast<const unsigned char*>(m_Buffer->GetImageBufferData())) {
					if (const class advancedfx::CImageFormat* pFormat = m_Buffer->GetImageBufferFormat()) {
						m_InFormat = *pFormat;
						if (m_InFormat.Format == advancedfx::ImageFormat::ZFloat) {
							m_pInData = pData;
							m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
							m_OutFormat.SetOrigin(m_InFormat.Origin);
							CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
							if (pOutBuffer) {
								m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
							}
							return pOutBuffer;
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

		IImageBufferThreadSafe* m_Buffer;
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
		CTransformDepth24(IImageBufferThreadSafe* buffer, float depthScale, float depthOfs)
			: m_Buffer(buffer)
			, m_DepthScale(depthScale)
			, m_DepthOfs(depthOfs)
		{
		}

		virtual IImageBufferThreadSafe* CreateOutput(CGrowingBufferPoolThreadSafe * imageBufferPool) {
			if (nullptr != m_Buffer) {
				if (const unsigned char* pData = static_cast<const unsigned char*>(m_Buffer->GetImageBufferData())) {
					if (const class advancedfx::CImageFormat* pFormat = m_Buffer->GetImageBufferFormat()) {
						m_InFormat = *pFormat;
						if (m_InFormat.Format == advancedfx::ImageFormat::BGR || m_InFormat.Format == advancedfx::ImageFormat::BGRA) {
							m_pInData = pData;
							m_OutFormat = advancedfx::CImageFormat(advancedfx::ImageFormat::ZFloat, m_InFormat.Width, m_InFormat.Height);
							m_OutFormat.SetOrigin(m_InFormat.Origin);
							CImageBufferThreadSafe* pOutBuffer = AquireFormatedImageBuffer(imageBufferPool, m_OutFormat);
							if (pOutBuffer) {
								m_pOutData = static_cast<unsigned char*>(pOutBuffer->GetImageBufferData());
							}
							return pOutBuffer;
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

		IImageBufferThreadSafe* m_Buffer;
		advancedfx::CImageFormat m_InFormat;
		const unsigned char* m_pInData;
		advancedfx::CImageFormat m_OutFormat;
		unsigned char* m_pOutData;
		float m_DepthScale;
		float m_DepthOfs;
	};

IImageBufferThreadSafe* Transform(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, class ITransform* transform) {
    if (IImageBufferThreadSafe* pOutBuffer = transform->CreateOutput(imageBufferPool)) {
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

        return pOutBuffer;
    }

    return nullptr;
}

/*
static IImageBufferThreadSafe* DummyBuffer() {
    advancedfx::CImageFormat format(advancedfx::ImageFormat::BGRA, 1280, 720);
    format.SetOrigin(advancedfx::ImageOrigin::TopLeft);
    CImageBufferThreadSafe * result = AquireFormatedImageBuffer(format);
    result->AddRef();
    return result;
}
*/

IImageBufferThreadSafe* StripAlpha(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer) {

    if (nullptr == buffer) return nullptr;

	if (const class advancedfx::CImageFormat* pFormat = buffer->GetImageBufferFormat()) {
		if (pFormat->Format == advancedfx::ImageFormat::BGR) {
			buffer->AddRef();
			return buffer;
		}
	}

	CTransformStripAlpha transform(buffer);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* RgbaToBgr(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer) {
    CTransformRgbaToBgr transform(buffer);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* RgbaToBgra(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer) {
    CTransformRgbaToBgra transform(buffer);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* DepthF(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer, float depthScale, float depthOfs) {
    CTransformDepthF transform(buffer, depthScale, depthOfs);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* Depth24(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* buffer, float depthScale, float depthOfs) {
    CTransformDepth24 transform(buffer, depthScale, depthOfs);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* Matte(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* bufferEntBlack, IImageBufferThreadSafe* bufferEntWhite) {
    CTransformMatte transform(bufferEntBlack, bufferEntWhite);
    return Transform(threadPool, imageBufferPool, &transform);
}

IImageBufferThreadSafe* AColorBRedAsAlpha(class CThreadPool * threadPool, CGrowingBufferPoolThreadSafe * imageBufferPool, IImageBufferThreadSafe* aColor, IImageBufferThreadSafe* bRedAsAlpha) {
    CTransformAColorBRedAsAlpha transform(aColor, bRedAsAlpha);
    return Transform(threadPool, imageBufferPool, &transform);
}

} // namespace ImageTransformer {
} // namespace advancedfx

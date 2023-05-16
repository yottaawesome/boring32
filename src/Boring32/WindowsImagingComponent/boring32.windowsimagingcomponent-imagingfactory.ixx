module;

#include <source_location>;

export module boring32.windowsimagingcomponent:imagingfactory;
import <string>;
import <stdexcept>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::WindowsImagingComponent
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
	class ImagingFactory final
	{
		public:
			virtual ~ImagingFactory()
			{
				Close();
			}

			ImagingFactory()
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
				const HRESULT hr = CoCreateInstance(
					CLSID_WICImagingFactory,
					nullptr,
					CLSCTX_INPROC_SERVER,
					IID_PPV_ARGS(&m_imagingFactory)
				);
				if (FAILED(hr))
					throw Error::COMError("CoCreateInstance() failed", hr);
			}

			ImagingFactory(const ImagingFactory& other)
			{
				Copy(other);
			}

			ImagingFactory(ImagingFactory&& other) noexcept
			{
				Move(other);
			}

		public:
			ImagingFactory& operator=(const ImagingFactory& other)
			{
				return Copy(other);
			}

			ImagingFactory& operator=(ImagingFactory&& other) noexcept
			{
				return Move(other);
			}

		public:
			void Close()
			{
				m_imagingFactory = nullptr;
			}

			Microsoft::WRL::ComPtr<IWICBitmapDecoder> CreateDecoderFromFilename(
				const std::wstring& path
			)
			{
				if (path.empty())
					throw Error::Boring32Error("path cannot be empty");
				if (!m_imagingFactory)
					throw Error::Boring32Error("m_imagingFactory is nullptr");

				Microsoft::WRL::ComPtr<IWICBitmapDecoder> result;
				// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nf-wincodec-iwicimagingfactory-createdecoderfromfilename
				const HRESULT hr = m_imagingFactory->CreateDecoderFromFilename(
					path.c_str(),
					nullptr,
					GENERIC_READ,
					WICDecodeMetadataCacheOnLoad,
					&result
				);
				if (FAILED(hr))
					throw Error::COMError("CreateDecoderFromFilename() failed", hr);

				return result;
			}

			Microsoft::WRL::ComPtr<IWICFormatConverter> CreateFormatConverter()
			{
				if (!m_imagingFactory)
					throw Error::Boring32Error("m_imagingFactory is nullptr");

				Microsoft::WRL::ComPtr<IWICFormatConverter> pConverter;
				const HRESULT hr = m_imagingFactory->CreateFormatConverter(&pConverter);
				if (FAILED(hr))
					throw Error::COMError("CreateFormatConverter() failed", hr);

				return pConverter;
			}

		private:
			ImagingFactory& Copy(const ImagingFactory& other)
			{
				if (this == &other)
					return *this;

				m_imagingFactory = other.m_imagingFactory;

				return *this;
			}

			ImagingFactory& Move(const ImagingFactory& other)
			{
				if (this == &other)
					return *this;

				m_imagingFactory = std::move(other.m_imagingFactory);

				return *this;
			}

		private:
			Microsoft::WRL::ComPtr<IWICImagingFactory> m_imagingFactory;
	};
}
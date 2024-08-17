export module boring32:windowsimagingcomponent_imagingfactory;
import boring32.shared;
import :error;

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
				const Win32::HRESULT hr = Win32::CoCreateInstance(
					Win32::CLSID_WICImagingFactory2,
					nullptr,
					Win32::CLSCTX::CLSCTX_INPROC_SERVER,
					__uuidof (**(&m_imagingFactory)), 
					&m_imagingFactory
				);
				if (Win32::HrFailed(hr))
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

			Win32::ComPtr<Win32::IWICBitmapDecoder> CreateDecoderFromFilename(
				const std::wstring& path
			)
			{
				if (path.empty())
					throw Error::Boring32Error("path cannot be empty");
				if (!m_imagingFactory)
					throw Error::Boring32Error("m_imagingFactory is nullptr");

				Win32::ComPtr<Win32::IWICBitmapDecoder> result;
				// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nf-wincodec-iwicimagingfactory-createdecoderfromfilename
				const Win32::HRESULT hr = m_imagingFactory->CreateDecoderFromFilename(
					path.c_str(),
					nullptr,
					Win32::GenericRead,
					Win32::WICDecodeOptions::WICDecodeMetadataCacheOnLoad,
					&result
				);
				if (Win32::HrFailed(hr))
					throw Error::COMError("CreateDecoderFromFilename() failed", hr);

				return result;
			}

			Win32::ComPtr<Win32::IWICFormatConverter> CreateFormatConverter()
			{
				if (!m_imagingFactory)
					throw Error::Boring32Error("m_imagingFactory is nullptr");

				Win32::ComPtr<Win32::IWICFormatConverter> pConverter;
				const Win32::HRESULT hr = m_imagingFactory->CreateFormatConverter(&pConverter);
				if (Win32::HrFailed(hr))
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
			Win32::ComPtr<Win32::IWICImagingFactory> m_imagingFactory;
	};
}
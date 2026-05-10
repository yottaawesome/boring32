export module boring32:windowsimagingcomponent.imagingfactory;
import :win32;
import :error;
import :com;

export namespace Boring32::WindowsImagingComponent
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
	class ImagingFactory final
	{
	public:
		~ImagingFactory()
		{
			Close();
		}

		ImagingFactory()
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
			auto hr = 
				Win32::CoCreateInstance(
					Win32::CLSID_WICImagingFactory2,
					nullptr,
					Win32::CLSCTX::CLSCTX_INPROC_SERVER,
					m_imagingFactory.GetUuid(),
					m_imagingFactory.ReleaseAndGetAddressOf()
				);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "CoCreateInstance() failed");
		}

		// We can support copy semantics trivially, but for now, no compelling reason to do so.
		ImagingFactory(const ImagingFactory& other) = delete;
		auto operator=(const ImagingFactory& other) -> ImagingFactory& = delete;

		ImagingFactory(ImagingFactory&& other) noexcept
		{
			Move(other);
		}
		auto operator=(ImagingFactory&& other) noexcept -> ImagingFactory&
		{
			return Move(other);
		}

		void Close()
		{
			m_imagingFactory = nullptr;
		}

		auto CreateDecoderFromFilename(const std::wstring& path) -> Win32::ComPtr<Win32::IWICBitmapDecoder>
		{
			if (path.empty())
				throw Error::Boring32Error{"path cannot be empty"};
			if (not m_imagingFactory)
				throw Error::Boring32Error{"m_imagingFactory is nullptr"};

			auto result = Win32::ComPtr<Win32::IWICBitmapDecoder>{};
			// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nf-wincodec-iwicimagingfactory-createdecoderfromfilename
			auto hr = 
				m_imagingFactory->CreateDecoderFromFilename(
					path.c_str(),
					nullptr,
					Win32::GenericRead,
					Win32::WICDecodeOptions::WICDecodeMetadataCacheOnLoad,
					&result
				);
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "CreateDecoderFromFilename() failed"};

			return result;
		}

		auto CreateFormatConverter() -> Com::Ptr<Win32::IWICFormatConverter>
		{
			if (not m_imagingFactory)
				throw Error::Boring32Error{"m_imagingFactory is nullptr"};

			auto converter = Com::Ptr<Win32::IWICFormatConverter>{};
			auto hr = m_imagingFactory->CreateFormatConverter(converter.ReleaseAndGetAddressOfTyped());
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "CreateFormatConverter() failed"};

			return converter;
		}

	private:
		auto Move(const ImagingFactory& other) -> ImagingFactory&
		{
			if (this == &other)
				return *this;

			m_imagingFactory = std::move(other.m_imagingFactory);

			return *this;
		}

		Com::Ptr<Win32::IWICImagingFactory> m_imagingFactory;
	};
}
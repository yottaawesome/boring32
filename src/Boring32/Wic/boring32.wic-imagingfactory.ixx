module;

#include <string>
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

export module boring32.wic:imagingfactory;

export namespace Boring32::Wic
{
	// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
	class ImagingFactory
	{
		public:
			virtual ~ImagingFactory();
			ImagingFactory();
			ImagingFactory(const ImagingFactory& other);
			ImagingFactory(ImagingFactory&& other) noexcept;

		public:
			virtual ImagingFactory& operator=(const ImagingFactory& other);
			virtual ImagingFactory& operator=(ImagingFactory&& other) noexcept;

		public:
			virtual void Close();
			virtual Microsoft::WRL::ComPtr<IWICBitmapDecoder> CreateDecoderFromFilename(const std::wstring& path);
			virtual Microsoft::WRL::ComPtr<IWICFormatConverter> CreateFormatConverter();

		protected:
			virtual ImagingFactory& Copy(const ImagingFactory& other);
			virtual ImagingFactory& Move(const ImagingFactory& other);

		protected:
			Microsoft::WRL::ComPtr<IWICImagingFactory> m_imagingFactory;
	};
}
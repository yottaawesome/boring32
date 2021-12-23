module;

#include <stdexcept>
#include <string>
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

module boring32.wic.imagingfactory;
import boring32.error.comerror;

namespace Boring32::Wic
{
	ImagingFactory::~ImagingFactory()
	{
		Close();
	}

	ImagingFactory::ImagingFactory()
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wincodec/nn-wincodec-iwicimagingfactory
		const HRESULT hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_imagingFactory)
		);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__": CoCreateInstance() failed", hr);
	}

	void ImagingFactory::Close()
	{
		m_imagingFactory = nullptr;
	}

	Microsoft::WRL::ComPtr<IWICBitmapDecoder> ImagingFactory::CreateDecoderFromFilename(const std::wstring& path)
	{
		if (!m_imagingFactory)
			throw std::runtime_error(__FUNCSIG__": m_imagingFactory is nullptr");

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
			throw Error::ComError(__FUNCSIG__": CreateDecoderFromFilename() failed", hr);

		return result;
	}

	Microsoft::WRL::ComPtr<IWICFormatConverter> ImagingFactory::CreateFormatConverter()
	{
		if (!m_imagingFactory)
			throw std::runtime_error(__FUNCSIG__": m_imagingFactory is nullptr");

		Microsoft::WRL::ComPtr<IWICFormatConverter> pConverter;
		HRESULT hr = m_imagingFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": CreateFormatConverter() failed", hr);

		return pConverter;
	}
}
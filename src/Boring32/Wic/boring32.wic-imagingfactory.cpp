module;

#include <stdexcept>
#include <string>
#include <source_location>
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

module boring32.wic:imagingfactory;
import boring32.error;

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
			throw Error::ComError(std::source_location::current(), "CoCreateInstance() failed", hr);
	}

	ImagingFactory::ImagingFactory(const ImagingFactory& other)
	{
		Copy(other);
	}

	ImagingFactory::ImagingFactory(ImagingFactory&& other) noexcept
	{
		Move(other);
	}

	ImagingFactory& ImagingFactory::operator=(const ImagingFactory& other)
	{
		return Copy(other);
	}

	ImagingFactory& ImagingFactory::operator=(ImagingFactory&& other) noexcept
	{
		return Move(other);
	}

	void ImagingFactory::Close()
	{
		m_imagingFactory = nullptr;
	}

	ImagingFactory& ImagingFactory::Copy(const ImagingFactory& other)
	{
		if (this == &other)
			return *this;

		m_imagingFactory = other.m_imagingFactory;

		return *this;
	}

	ImagingFactory& ImagingFactory::Move(const ImagingFactory& other)
	{
		if (this == &other)
			return *this;

		m_imagingFactory = std::move(other.m_imagingFactory);

		return *this;
	}

	Microsoft::WRL::ComPtr<IWICBitmapDecoder> ImagingFactory::CreateDecoderFromFilename(const std::wstring& path)
	{
		if (path.empty())
			throw std::invalid_argument(__FUNCSIG__": path cannot be empty");
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
			throw Error::ComError(std::source_location::current(), "CreateDecoderFromFilename() failed", hr);

		return result;
	}

	Microsoft::WRL::ComPtr<IWICFormatConverter> ImagingFactory::CreateFormatConverter()
	{
		if (!m_imagingFactory)
			throw std::runtime_error(__FUNCSIG__": m_imagingFactory is nullptr");

		Microsoft::WRL::ComPtr<IWICFormatConverter> pConverter;
		const HRESULT hr = m_imagingFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr))
			throw Error::ComError(std::source_location::current(), "CreateFormatConverter() failed", hr);

		return pConverter;
	}
}
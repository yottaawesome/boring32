export module boring32:com.functions;
import std;
import :win32;

export namespace Boring32::Com
{
	constexpr auto Succeeded(const Win32::HRESULT hr) noexcept -> bool
	{
		return hr >= 0;
	}

	constexpr auto Failed(const Win32::HRESULT hr) noexcept -> bool
	{
		return hr < 0;
	}

	constexpr auto Facility(const Win32::HRESULT hr) noexcept -> long
	{
		// HRESULT_FACILITY
		return (hr >> 16) & 0x1fff;
	}

	constexpr auto Code(const Win32::HRESULT hr) noexcept -> long
	{
		// HRESULT_CODE
		return hr & 0xFFFF;
	}

	constexpr auto Severity(const Win32::HRESULT hr) noexcept -> long
	{
		// HRESULT_SEVERITY
		return (hr >> 31) & 0x1;
	}
}

export module boring32:com_functions;
import std;
import boring32.win32;

export namespace Boring32::COM
{
	constexpr bool Succeeded(const Win32::HRESULT hr) noexcept
	{
		return hr >= 0;
	}

	constexpr bool Failed(const Win32::HRESULT hr) noexcept
	{
		return hr < 0;
	}

	constexpr long Facility(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_FACILITY
		return (hr >> 16) & 0x1fff;
	}

	constexpr long Code(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_CODE
		return hr & 0xFFFF;
	}

	constexpr long Severity(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_SEVERITY
		return (hr >> 31) & 0x1;
	}
}

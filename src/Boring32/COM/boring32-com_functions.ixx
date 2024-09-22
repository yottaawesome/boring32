export module boring32:com_functions;
import boring32.shared;

export namespace Boring32::COM
{
	inline constexpr bool Succeeded(const Win32::HRESULT hr) noexcept
	{
		return hr >= 0;
	}

	inline constexpr bool Failed(const Win32::HRESULT hr) noexcept
	{
		return hr < 0;
	}

	inline constexpr long Facility(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_FACILITY
		return (hr >> 16) & 0x1fff;
	}

	inline constexpr long Code(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_CODE
		return hr & 0xFFFF;
	}

	inline constexpr long Severity(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_SEVERITY
		return (hr >> 31) & 0x1;
	}
}

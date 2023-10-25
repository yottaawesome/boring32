export module boring32.com:functions;
import boring32.win32;

export namespace Boring32::COM
{
	inline bool Succeeded(const Win32::HRESULT hr) noexcept
	{
		return hr >= 0;
	}

	inline bool Failed(const Win32::HRESULT hr) noexcept
	{
		return hr < 0;
	}

	inline long Facility(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_FACILITY
		return (hr >> 16) & 0x1fff;
	}

	inline long Code(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_CODE
		return hr & 0xFFFF;
	}

	inline long Severity(const Win32::HRESULT hr) noexcept
	{
		// HRESULT_SEVERITY
		return (hr >> 31) & 0x1;
	}
}

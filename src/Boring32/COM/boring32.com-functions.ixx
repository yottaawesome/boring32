module;

#include <source_location>;

export module boring32.com:functions;
import <Win32.hpp>;

export namespace Boring32::COM
{
	inline bool Succeeded(const HRESULT hr) noexcept
	{
		return hr >= 0;
	}

	inline bool Failed(const HRESULT hr) noexcept
	{
		return hr < 0;
	}

	inline long Facility(const HRESULT hr) noexcept
	{
		// HRESULT_FACILITY
		return (hr >> 16) & 0x1fff;
	}

	inline long Code(const HRESULT hr) noexcept
	{
		// HRESULT_CODE
		return hr & 0xFFFF;
	}

	inline long Severity(const HRESULT hr) noexcept
	{
		// HRESULT_SEVERITY
		return (hr >> 31) & 0x1;
	}
}

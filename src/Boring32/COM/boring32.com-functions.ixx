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
}

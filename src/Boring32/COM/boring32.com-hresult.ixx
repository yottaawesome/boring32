module;

#include <source_location>

export module boring32.com:hresult;
import <win32.hpp>;

export namespace Boring32::COM
{
	// See https://learn.microsoft.com/en-us/windows/win32/com/error-handling-in-com
	class HResult
	{
		public:
			virtual ~HResult() noexcept = default;
			HResult() = default;
			HResult(HResult&&) noexcept = default;
			HResult(const HResult&) noexcept = default;
			HResult(const HRESULT hr) noexcept;
			HResult(
				const long severity, 
				const long facility, 
				const long code
			) noexcept;

		public:
			virtual operator HRESULT() const noexcept;
			virtual HResult& operator=(const HRESULT hr) noexcept;
			virtual operator bool() const noexcept;

		// See https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
		public:
			virtual HRESULT Get() const noexcept;
			virtual long Facility() const noexcept;
			virtual long Code() const noexcept;
			virtual long Severity() const noexcept;
			virtual void ThrowIfFailed(
				const char* msg,
				const std::source_location& loc = std::source_location::current()
			) const;

		protected:
			HRESULT m_hr = 0x0;
	};
}

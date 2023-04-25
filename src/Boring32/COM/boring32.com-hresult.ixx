export module boring32.com:hresult;
import :functions;
import boring32.error;
import <source_location>;
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
			HResult(const HRESULT hr) noexcept
				: m_hr(hr)
			{ }
			HResult(
				const long severity, 
				const long facility, 
				const long code
			) noexcept
			{
				m_hr = MAKE_HRESULT(severity, facility, code);
			}

		public:
			virtual operator HRESULT() const noexcept
			{
				return m_hr;
			}

			virtual HResult& operator=(const HRESULT hr) noexcept
			{
				m_hr = hr;
				return *this;
			}

			virtual operator bool() const noexcept
			{
				return Succeeded(m_hr);
			}

			virtual bool operator==(const HRESULT hr) const noexcept
			{
				return m_hr == hr;
			}

			virtual bool operator==(const HResult& hr) const noexcept
			{
				return m_hr == hr.m_hr;
			}

		// See https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
		public:
			virtual HRESULT Get() const noexcept
			{
				return m_hr;
			}

			virtual long Facility() const noexcept
			{
				return HRESULT_FACILITY(m_hr);
			}

			virtual long Code() const noexcept
			{
				return HRESULT_CODE(m_hr);
			}

			virtual long Severity() const noexcept
			{
				return HRESULT_SEVERITY(m_hr);
			}

			virtual void ThrowIfFailed(
				const char* msg,
				const std::source_location& loc = std::source_location::current()
			) const
			{
				if (Succeeded(m_hr))
					return;
				if (msg)
					throw Error::COMError(msg, m_hr, loc);
				throw Error::COMError("HRESULT check failed", m_hr, loc);
			}

		protected:
			HRESULT m_hr = 0x0;
	};
}

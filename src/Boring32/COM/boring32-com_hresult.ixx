export module boring32:com_hresult;
import std;
import boring32.win32;
import :error;
import :com_functions;

export namespace Boring32::COM
{
	// See https://learn.microsoft.com/en-us/windows/win32/com/error-handling-in-com
	struct HResult final
	{
		constexpr HResult() noexcept = default;

		constexpr HResult(const Win32::HRESULT hr) noexcept
			: m_hr(hr) { }

		constexpr HResult(const long severity, const long facility, const long code) noexcept
		{
			m_hr = Win32::MakeHResult(severity, facility, code);
		}

		constexpr operator Win32::HRESULT() const noexcept { return m_hr; }

		constexpr HResult& operator=(const Win32::HRESULT hr) noexcept
		{
			m_hr = hr;
			return *this;
		}

		constexpr operator bool() const noexcept { return Succeeded(); }

		constexpr bool operator==(const Win32::HRESULT hr) const noexcept { return m_hr == hr; }

		constexpr bool operator==(const HResult& hr) const noexcept { return m_hr == hr.m_hr; }

		// See https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
		constexpr HRESULT Get() const noexcept { return m_hr; }

		constexpr long Facility() const noexcept { return Win32::Facility(m_hr); }

		constexpr long Code() const noexcept { return Win32::Code(m_hr); }

		constexpr long Severity() const noexcept { return Win32::Severity(m_hr); }

		constexpr bool Succeeded() const noexcept { return COM::Succeeded(m_hr); }

		constexpr bool Failed() const noexcept { return not Succeeded(); }

		void ThrowIfFailed(std::string_view msg, const std::source_location& loc = std::source_location::current()) const
		{
			if (Succeeded())
				return;
			if (not msg.empty())
				throw Error::COMError(m_hr, msg.data(), loc);
			throw Error::COMError(m_hr, "HRESULT check failed", loc);
		}

		private:
		Win32::HRESULT m_hr = 0x0;
	};
}

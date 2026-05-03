export module boring32:com.hresult;
import std;
import :win32;
import :error;
import :com.functions;

export namespace Boring32::Com
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

		constexpr auto operator==(const Win32::HRESULT hr) const noexcept -> bool { return m_hr == hr; }

		constexpr auto operator==(const HResult& hr) const noexcept -> bool { return m_hr == hr.m_hr; }

		// See https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
		constexpr auto Get() const noexcept -> Win32::HRESULT { return m_hr; }

		constexpr auto Facility() const noexcept -> long { return Win32::Facility(m_hr); }

		constexpr auto Code() const noexcept -> long { return Win32::Code(m_hr); }

		constexpr auto Severity() const noexcept -> long { return Win32::Severity(m_hr); }

		constexpr auto Succeeded() const noexcept -> bool { return Com::Succeeded(m_hr); }

		constexpr auto Failed() const noexcept -> bool { return not Succeeded(); }

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

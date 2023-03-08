module boring32.com:hresult;
import :functions;
import boring32.error;

namespace Boring32::COM
{
	HResult::HResult(const HRESULT hr) noexcept
		: m_hr(hr)
	{ }

	HResult::HResult(
		const long severity,
		const long facility,
		const long code
	) noexcept
	{
		m_hr = MAKE_HRESULT(severity, facility, code);
	}

	HResult::operator HRESULT() const noexcept
	{
		return m_hr;
	}

	HResult& HResult::operator=(const HRESULT hr) noexcept
	{
		m_hr = hr;
		return *this;
	}

	HResult::operator bool() const noexcept
	{
		return Succeeded(m_hr);
	}

	HRESULT HResult::Get() const noexcept
	{
		return m_hr;
	}

	long HResult::Facility() const noexcept
	{
		return HRESULT_FACILITY(m_hr);
	}

	long HResult::Code() const noexcept
	{
		return HRESULT_CODE(m_hr);
	}

	long HResult::Severity() const noexcept
	{
		return HRESULT_SEVERITY(m_hr);
	}

	void HResult::ThrowIfFailed(
		const char* msg,
		const std::source_location& loc
	) const
	{
		if (Succeeded(m_hr))
			return;
		if (msg)
			throw Error::COMError(msg, m_hr, loc);
		throw Error::COMError("HRESULT check failed", m_hr, loc);
	}

	bool HResult::operator==(const HRESULT hr) const noexcept
	{
		return m_hr == hr;
	}

	bool HResult::operator==(const HResult& hr) const noexcept
	{
		return m_hr == hr.m_hr;
	}
}

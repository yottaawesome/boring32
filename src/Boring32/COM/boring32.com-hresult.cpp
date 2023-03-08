module boring32.com:hresult;

namespace Boring32::COM
{
	HResult::HResult(const HRESULT hr)
		: m_hr(hr)
	{ }

	HResult::operator HRESULT() const noexcept
	{
		return m_hr;
	}

	HResult& HResult::operator=(const HRESULT hr) noexcept
	{
		m_hr = hr;
		return *this;
	}
}

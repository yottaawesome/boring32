module;

#include <source_location>

module boring32.async:waitableaddress;
import boring32.error;

namespace Boring32::Async
{
	WaitableAddress::~WaitableAddress() {}
	WaitableAddress::WaitableAddress() : m_defaultValue(0) {}
	WaitableAddress::WaitableAddress(const DWORD defaultValue, const DWORD waitValue)
		: m_defaultValue(defaultValue), m_waitValue(waitValue)
	{ }

	bool WaitableAddress::Wait() const
	{
		return Wait(INFINITE);
	}

	bool WaitableAddress::Wait(const DWORD millis) const
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
		const bool waitSucceeded = WaitOnAddress(
			const_cast<unsigned long*>(&m_waitValue),
			const_cast<unsigned long*>(&m_defaultValue),
			sizeof(DWORD), // The size of the value, in bytes.This parameter can be 1, 2, 4, or 8.
			millis
		);
		if (!waitSucceeded)
		{
			const auto lastError = GetLastError();
			if (lastError == ERROR_TIMEOUT)
				return false;
			throw Error::Win32Error("WaitOnAddress() failed", lastError);
		}
		return true;
	}

	void WaitableAddress::SetValue(const DWORD newValue, const WaitableAddress::WakeType wakeType)
	{
		m_waitValue = newValue;
		switch (wakeType)
		{
			case WaitableAddress::WakeType::One:
				// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddresssingle
				WakeByAddressSingle(&m_waitValue);
				break;

			case WaitableAddress::WakeType::All:
				WakeByAddressAll(&m_waitValue);
				break;

			default:
				throw Error::Boring32Error("Unknown wakeType");
		}
	}

	DWORD WaitableAddress::GetValue() const noexcept
	{
		return m_waitValue;
	}

	DWORD WaitableAddress::GetDefaultValue() const noexcept
	{
		return m_defaultValue;
	}
}
export module boring32:async_waitableaddress;
import boring32.win32;
import :error;

export namespace Boring32::Async
{
	// TODO: need to support values that can be 1, 2, 4, or 8 bytes long
	struct WaitableAddress final
	{
		enum class WakeType
		{
			One,
			All
		};

		WaitableAddress() = default;
		WaitableAddress(const WaitableAddress& other) = delete;
		WaitableAddress(WaitableAddress&& other) noexcept = delete;
		WaitableAddress(const Win32::DWORD defaultValue, const Win32::DWORD waitValue)
			: m_defaultValue(defaultValue), m_waitValue(waitValue)
		{ }

		WaitableAddress& operator=(const WaitableAddress& other) = delete;
		WaitableAddress& operator=(WaitableAddress&& other) noexcept = delete;

		bool Wait() const
		{
			return Wait(Win32::Infinite);
		}

		bool Wait(const Win32::DWORD millis) const
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
			bool waitSucceeded = Win32::WaitOnAddress(
				const_cast<unsigned long*>(&m_waitValue),
				const_cast<unsigned long*>(&m_defaultValue),
				sizeof(Win32::DWORD), // The size of the value, in bytes.This parameter can be 1, 2, 4, or 8.
				millis
			);
			if (auto lastError = Win32::GetLastError(); not waitSucceeded)
			{
				if (lastError == Win32::ErrorCodes::Timeout)
					return false;
				throw Error::Win32Error("WaitOnAddress() failed", lastError);
			}
			return true;
		}

		void SetValue(const Win32::DWORD newValue, const WakeType wakeType)
		{
			m_waitValue = newValue;
			switch (wakeType)
			{
				case WaitableAddress::WakeType::One:
					// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddresssingle
					Win32::WakeByAddressSingle(&m_waitValue);
					break;

				case WaitableAddress::WakeType::All:
					Win32::WakeByAddressAll(&m_waitValue);
					break;

				default:
					throw Error::Boring32Error("Unknown wakeType");
			}
		}

		Win32::DWORD GetValue() const noexcept
		{
			return m_waitValue;
		}

		Win32::DWORD GetDefaultValue() const noexcept
		{
			return m_defaultValue;
		}

		private:
		// These might probably be best extracted into their own template class
		Win32::DWORD m_defaultValue = 0;
		Win32::DWORD m_waitValue = 0;
	};
}
export module boring32.async:waitableaddress;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Async
{
	// TODO: need to support values that can be 1, 2, 4, or 8 bytes long
	class WaitableAddress
	{
		public:
			enum class WakeType
			{
				One,
				All
			};

		public:
			virtual ~WaitableAddress() = default;
			WaitableAddress() = default;
			WaitableAddress(const WaitableAddress& other) = delete;
			WaitableAddress(WaitableAddress&& other) noexcept = delete;
			WaitableAddress(const DWORD defaultValue, const DWORD waitValue)
				: m_defaultValue(defaultValue), m_waitValue(waitValue)
			{ }

		public:
			virtual WaitableAddress& operator=(const WaitableAddress& other) = delete;
			virtual WaitableAddress& operator=(WaitableAddress&& other) noexcept = delete;

		public:
			virtual bool Wait() const
			{
				return Wait(INFINITE);
			}

			virtual bool Wait(const DWORD millis) const
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

			virtual void SetValue(const DWORD newValue, const WakeType wakeType)
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

			virtual DWORD GetValue() const noexcept
			{
				return m_waitValue;
			}

			virtual DWORD GetDefaultValue() const noexcept
			{
				return m_defaultValue;
			}

		protected:
			// These might probably be best extracted into their own template class
			DWORD m_defaultValue = 0;
			DWORD m_waitValue = 0;
	};
}
export module boring32:crypto_cryptokey;
import boring32.shared;

export namespace Boring32::Crypto
{
	std::shared_ptr<void> Generate(Win32::BCRYPT_KEY_HANDLE ptr)
	{
		return std::shared_ptr<void>{ ptr, Win32::BCryptDestroyKey };
	}
}

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
export namespace Boring32::Crypto
{
	class CryptoKey final
	{
		public:
			~CryptoKey()
			{
				Close();
			}

			CryptoKey() = default;
			CryptoKey(const CryptoKey&) = delete;
			CryptoKey& operator=(const CryptoKey&) = delete;

			CryptoKey(CryptoKey&& other) noexcept
				: m_keyHandle(nullptr)
			{
				Move(other);
			}

			CryptoKey& operator=(CryptoKey&& other) noexcept
			{
				return Move(other);
			}

		public:
			CryptoKey(
				Win32::BCRYPT_KEY_HANDLE const keyHandle,
				std::vector<std::byte>&& keyObject
			) : m_keyHandle(keyHandle),
				m_keyObject(std::move(keyObject))
			{ }

		public:
			BCRYPT_KEY_HANDLE GetHandle() const noexcept
			{
				return m_keyHandle;
			}

			void Close()
			{
				if (m_keyHandle)
				{
					Win32::BCryptDestroyKey(m_keyHandle);
					m_keyHandle = nullptr;
				}
			}

		private:
			CryptoKey& Move(CryptoKey& other) noexcept
			{
				Close();
				m_keyHandle = other.m_keyHandle;
				other.m_keyHandle = nullptr;
				m_keyObject = std::move(other.m_keyObject);
				return *this;
			}

		private:
			// We can duplicate this key, but for now, just share it
			Win32::BCRYPT_KEY_HANDLE m_keyHandle = nullptr;
			std::vector<std::byte> m_keyObject;
	};
}
export module boring32:crypto.cryptokey;
import std;
import :win32;

export namespace Boring32::Crypto
{
	auto Generate(Win32::BCRYPT_KEY_HANDLE ptr) -> std::shared_ptr<void>
	{
		return std::shared_ptr<void>{ ptr, Win32::BCryptDestroyKey };
	}
}

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
export namespace Boring32::Crypto
{
	struct CryptoKey final
	{
		~CryptoKey()
		{
			Close();
		}

		CryptoKey() = default;
		CryptoKey(const CryptoKey&) = delete;
		CryptoKey& operator=(const CryptoKey&) = delete;

		CryptoKey(CryptoKey&& other) noexcept
		{
			Move(other);
		}

		auto operator=(CryptoKey&& other) noexcept -> CryptoKey&
		{
			return Move(other);
		}

		CryptoKey(Win32::BCRYPT_KEY_HANDLE const keyHandle, std::vector<std::byte>&& keyObject) 
			: m_keyHandle(keyHandle),
			m_keyObject(std::move(keyObject))
		{ }

		auto GetHandle() const noexcept -> Win32::BCRYPT_KEY_HANDLE
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
		auto Move(CryptoKey& other) noexcept -> CryptoKey&
		{
			Close();
			m_keyHandle = other.m_keyHandle;
			other.m_keyHandle = nullptr;
			m_keyObject = std::move(other.m_keyObject);
			return *this;
		}

		// We can duplicate this key, but for now, just share it
		Win32::BCRYPT_KEY_HANDLE m_keyHandle = nullptr;
		std::vector<std::byte> m_keyObject;
	};
}
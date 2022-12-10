module;

#include <string>
#include <vector>
#include <memory>

export module boring32.crypto:cryptokey;
import <win32.hpp>;

export namespace Boring32::Crypto
{
	class CryptoKey
	{
		public:
			virtual ~CryptoKey();
			CryptoKey();
			CryptoKey(BCRYPT_KEY_HANDLE const keyHandle, std::vector<std::byte>&& keyObject);

			CryptoKey(const CryptoKey&) = delete;
			virtual CryptoKey& operator=(const CryptoKey&) = delete;

			CryptoKey(CryptoKey&& other) noexcept;
			virtual CryptoKey& operator=(CryptoKey&& other) noexcept;

		public:
			virtual BCRYPT_KEY_HANDLE GetHandle() const noexcept;
			virtual void Close();

		protected:
			virtual CryptoKey& Move(CryptoKey& other) noexcept;

		protected:
			// We can duplicate this key, but for now, just share it
			BCRYPT_KEY_HANDLE m_keyHandle;
			std::vector<std::byte> m_keyObject;
	};
}
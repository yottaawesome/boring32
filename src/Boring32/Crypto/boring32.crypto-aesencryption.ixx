module;

#include <string>
#include <vector>
#include <Windows.h>
#include <bcrypt.h>

export module boring32.crypto:aesencryption;
import :cryptokey;
import :chainingmode;

export namespace Boring32::Crypto
{
	class AesEncryption
	{
		public:
			virtual ~AesEncryption();
			AesEncryption();

			AesEncryption(const AesEncryption& other);
			virtual AesEncryption& operator=(const AesEncryption& other);

			AesEncryption(AesEncryption&& other) noexcept;
			virtual AesEncryption& operator=(AesEncryption&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual BCRYPT_ALG_HANDLE GetHandle() const noexcept;
			virtual DWORD GetObjectByteSize() const;
			virtual DWORD GetBlockByteLength() const;
			virtual void SetChainingMode(const ChainingMode mode);
			virtual CryptoKey GenerateSymmetricKey(const std::vector<std::byte>& key);

			// IV will be modified during encryption, so pass a copy if needed
			virtual std::vector<std::byte> Encrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::wstring_view string
			);

			virtual std::vector<std::byte> Encrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::vector<std::byte>& data
			);

			virtual std::vector<std::byte> Decrypt(
				const CryptoKey& key,
				const std::vector<std::byte>& iv,
				const std::vector<std::byte>& cypherText
			);

		protected:
			virtual AesEncryption& Copy(const AesEncryption& other);
			virtual AesEncryption& Move(AesEncryption& other) noexcept;
			virtual void Create();
			virtual DWORD GetEncryptDecryptFlags() const;

		protected:
			BCRYPT_ALG_HANDLE m_algHandle;
			ChainingMode m_chainingMode;
	}; 
}
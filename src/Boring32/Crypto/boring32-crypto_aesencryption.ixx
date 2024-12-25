export module boring32:crypto_aesencryption;
import boring32.shared;
import :error;
import :crypto_cryptokey;
import :crypto_chainingmode;

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
export namespace Boring32::Crypto
{
	struct AesEncryption final
	{
		~AesEncryption()
		{
			Close();
		}

		AesEncryption()
			: m_chainingMode(ChainingMode::CipherBlockChaining)
		{
			Create();
		}

		AesEncryption(const AesEncryption& other)
		{
			Copy(other);
		}

		AesEncryption& operator=(const AesEncryption& other)
		{
			return Copy(other);
		}

		AesEncryption(AesEncryption&& other) noexcept
		{
			Move(other);
		}
			
		AesEncryption& operator=(AesEncryption&& other) noexcept
		{
			return Move(other);
		}

		void Close() noexcept
		{
			if (m_algHandle)
			{
				//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptclosealgorithmprovider
				Win32::BCryptCloseAlgorithmProvider(m_algHandle, 0);
				m_algHandle = nullptr;
				m_chainingMode = ChainingMode::CipherBlockChaining;
			}
		}

		Win32::BCRYPT_ALG_HANDLE GetHandle() const noexcept
		{
			return m_algHandle;
		}

		Win32::DWORD GetObjectByteSize() const
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");

			Win32::DWORD cbKeyObject = 0;
			Win32::DWORD cbData = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
			Win32::NTSTATUS status = Win32::BCryptGetProperty(
				m_algHandle,
				Win32::_BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<Win32::PBYTE>(&cbKeyObject),
				sizeof(Win32::DWORD),
				&cbData,
				0
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "Failed to set AES key length");

			return cbKeyObject;
		}

		Win32::DWORD GetBlockByteLength() const
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");

			Win32::DWORD cbKeyObject = 0;
			Win32::DWORD cbData = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
			Win32::NTSTATUS status = Win32::BCryptGetProperty(
				m_algHandle,
				Win32::_BCRYPT_BLOCK_LENGTH,
				reinterpret_cast<Win32::PBYTE>(&cbKeyObject),
				sizeof(Win32::DWORD),
				&cbData,
				0
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "Failed to set AES key length");

			return cbKeyObject;
		}

		void SetChainingMode(const ChainingMode cm)
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");

			const std::wstring& mode = ChainingModeToString(cm);
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptsetproperty
			Win32::NTSTATUS status = Win32::BCryptSetProperty(
				m_algHandle,
				Win32::_BCRYPT_CHAINING_MODE,
				reinterpret_cast<Win32::PUCHAR>(const_cast<wchar_t*>(&mode[0])),
				static_cast<Win32::ULONG>(mode.size() * sizeof(wchar_t)),
				0
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "Failed to set chaining mode");
			m_chainingMode = cm;
		}

		CryptoKey GenerateSymmetricKey(const std::vector<std::byte>& rgbAES128Key)
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");
			if (rgbAES128Key.empty())
				throw Error::Boring32Error("rgbAES128Key is empty");

			Win32::BCRYPT_KEY_HANDLE hKey = nullptr;
			Win32::DWORD cbKeyObject = GetObjectByteSize();
			std::vector<std::byte> keyObject(cbKeyObject, static_cast<std::byte>(0));
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgeneratesymmetrickey
			Win32::NTSTATUS status = Win32::BCryptGenerateSymmetricKey(
				m_algHandle,
				&hKey,
				reinterpret_cast<Win32::PUCHAR>(&keyObject[0]),
				cbKeyObject,
				reinterpret_cast<Win32::PBYTE>(const_cast<std::byte*>(&rgbAES128Key[0])),
				(Win32::ULONG)rgbAES128Key.size(),
				0
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "Failed to set chaining mode");

			return CryptoKey(hKey, std::move(keyObject));
		}

		// IV will be modified during encryption, so pass a copy if needed
		std::vector<std::byte> Encrypt(const CryptoKey& key, const std::vector<std::byte>& iv, const std::wstring_view string)
		{
			const std::byte* buffer = reinterpret_cast<const std::byte*>(&string[0]);
			return Encrypt(
				key,
				iv,
				std::vector<std::byte>(buffer, buffer + string.size() * sizeof(wchar_t))
			);
		}

		std::vector<std::byte> Encrypt(const CryptoKey& key, const std::vector<std::byte>& iv, const std::vector<std::byte>& plainText)
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");
			if (not key.GetHandle())
				throw Error::Boring32Error("Key is null");

			// IV is optional
			Win32::PUCHAR pIV = nullptr;
			Win32::ULONG ivSize = 0;
			if (not iv.empty())
			{
				if (iv.size() != GetBlockByteLength())
					throw Error::Boring32Error("IV must be the same size as the AES block lenth");
				pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
				ivSize = static_cast<Win32::ULONG>(iv.size());
			}

			// Determine the byte size of the encrypted data
			Win32::DWORD cbData = 0;
			Win32::DWORD flags = GetEncryptDecryptFlags();
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
			Win32::NTSTATUS status = Win32::BCryptEncrypt(
				key.GetHandle(),
				reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&plainText[0])),
				static_cast<Win32::ULONG>(plainText.size()),
				nullptr,
				pIV,
				ivSize,
				nullptr,
				0,
				&cbData,
				flags
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "BCryptEncrypt() failed to count bytes");

			// Actually do the encryption
			std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
			status = Win32::BCryptEncrypt(
				key.GetHandle(),
				reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&plainText[0])),
				static_cast<Win32::ULONG>(plainText.size()),
				nullptr,
				pIV,
				ivSize,
				reinterpret_cast<Win32::PUCHAR>(&cypherText[0]),
				static_cast<Win32::ULONG>(cypherText.size()),
				&cbData,
				flags
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "BCryptEncrypt() failed to encrypt");

			return cypherText;
		}

		std::vector<std::byte> Decrypt(const CryptoKey& key, const std::vector<std::byte>& iv, const std::vector<std::byte>& cypherText)
		{
			if (not m_algHandle)
				throw Error::Boring32Error("Cipher algorithm not initialised");
			if (not key.GetHandle())
				throw Error::Boring32Error("Key is null");

			// IV is optional
			Win32::PUCHAR pIV = nullptr;
			Win32::ULONG ivSize = 0;
			if (not iv.empty())
			{
				if (iv.size() != GetBlockByteLength())
					throw Error::Boring32Error("IV must be the same size as the AES block lenth");
				pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
				ivSize = static_cast<Win32::ULONG>(iv.size());
			}

			// Determine the byte size of the decrypted data
			Win32::DWORD cbData = 0;
			const Win32::DWORD flags = GetEncryptDecryptFlags();
			// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
			Win32::NTSTATUS status = Win32::BCryptDecrypt(
				key.GetHandle(),
				reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
				static_cast<Win32::ULONG>(cypherText.size()),
				nullptr,
				pIV,
				ivSize,
				nullptr,
				0,
				&cbData,
				flags
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "BCryptDecrypt() failed to count bytes");

			// Actually do the decryption
			std::vector<std::byte> plainText(cbData, std::byte{ 0 });
			status = Win32::BCryptDecrypt(
				key.GetHandle(),
				reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
				static_cast<Win32::ULONG>(cypherText.size()),
				nullptr,
				pIV,
				ivSize,
				reinterpret_cast<PUCHAR>(&plainText[0]),
				static_cast<ULONG>(plainText.size()),
				&cbData,
				flags
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "BCryptDecrypt() failed to decrypt");

			plainText.resize(cbData);
			return plainText;
		}

		private:
		AesEncryption& Copy(const AesEncryption& other)
		{
			Close();
			if (other.m_algHandle == nullptr)
				return *this;
			m_chainingMode = other.m_chainingMode;
			Create();
			return *this;
		}

		AesEncryption& Move(AesEncryption& other) noexcept
		{
			Close();
			m_algHandle = other.m_algHandle;
			other.m_algHandle = nullptr;
			m_chainingMode = other.m_chainingMode;
			return *this;
		}

		void Create()
		{
			if (m_chainingMode == ChainingMode::NotSet)
				throw Error::Boring32Error("m_chainingMode is not set");

			//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
			Win32::NTSTATUS status = Win32::BCryptOpenAlgorithmProvider(
				&m_algHandle,
				Win32::_BCRYPT_AES_ALGORITHM, // https://docs.microsoft.com/en-us/windows/win32/seccng/cng-algorithm-identifiers
				nullptr,
				0
			);
			if (not Win32::BCryptSuccess(status))
				throw Error::NTStatusError(status, "Failed to create AES algorithm");
			SetChainingMode(m_chainingMode);
		}

		Win32::DWORD GetEncryptDecryptFlags() const
		{
			// BCRYPT_BLOCK_PADDING must not be used with the authenticated encryption modes(AES - CCM and AES - GCM)
			if (m_chainingMode == ChainingMode::NotSet)
				throw Error::Boring32Error("m_chainingMode is not set");
			if (m_chainingMode == ChainingMode::GaloisCounterMode)
				return 0;
			if (m_chainingMode == ChainingMode::CbcMac)
				return 0;
			return Win32::_BCRYPT_BLOCK_PADDING;
		}

		Win32::BCRYPT_ALG_HANDLE m_algHandle = nullptr;
		ChainingMode m_chainingMode = ChainingMode::NotSet;
	}; 
}
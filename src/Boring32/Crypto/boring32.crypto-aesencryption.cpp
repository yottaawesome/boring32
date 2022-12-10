module;

//#include <ntstatus.h>
#include <source_location>

module boring32.crypto:aesencryption;
import :chainingmode;
import boring32.error;

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
namespace Boring32::Crypto
{
	AesEncryption::~AesEncryption()
	{
		Close();
	}

	AesEncryption::AesEncryption()
	:	m_algHandle(nullptr),
		m_chainingMode(ChainingMode::CipherBlockChaining)
	{
		Create();
	}

	AesEncryption::AesEncryption(AesEncryption&& other) noexcept
	:	m_algHandle(nullptr),
		m_chainingMode(ChainingMode::NotSet)
	{
		Move(other);
	}

	AesEncryption& AesEncryption::operator=(AesEncryption&& other) noexcept
	{
		return Move(other);
	}

	AesEncryption& AesEncryption::Move(AesEncryption& other) noexcept
	{
		Close();
		m_algHandle = other.m_algHandle;
		other.m_algHandle = nullptr;
		m_chainingMode = other.m_chainingMode;
		return *this;
	}

	AesEncryption::AesEncryption(const AesEncryption& other)
	:	m_algHandle(nullptr)
	{
		Copy(other);
	}

	AesEncryption& AesEncryption::operator=(const AesEncryption& other)
	{
		return Copy(other);
	}

	AesEncryption& AesEncryption::Copy(const AesEncryption& other)
	{
		Close();
		if (other.m_algHandle == nullptr)
			return *this;
		m_chainingMode = other.m_chainingMode;
		Create();
		return *this;
	}

	void AesEncryption::Close() noexcept
	{
		if (m_algHandle)
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptclosealgorithmprovider
			BCryptCloseAlgorithmProvider(m_algHandle, 0);
			m_algHandle = nullptr;
			m_chainingMode = ChainingMode::CipherBlockChaining;
		}
	}

	void AesEncryption::Create()
	{
		if (m_chainingMode == ChainingMode::NotSet)
			throw Error::Boring32Error("m_chainingMode is not set");

		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
		const NTSTATUS status = BCryptOpenAlgorithmProvider(
			&m_algHandle,
			BCRYPT_AES_ALGORITHM, // https://docs.microsoft.com/en-us/windows/win32/seccng/cng-algorithm-identifiers
			nullptr,
			0
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("Failed to create AES algorithm", status);
		SetChainingMode(m_chainingMode);
	}

	BCRYPT_ALG_HANDLE AesEncryption::GetHandle() const noexcept
	{
		return m_algHandle;
	}

	DWORD AesEncryption::GetObjectByteSize() const
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");

		DWORD cbKeyObject = 0;
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
		const NTSTATUS status = BCryptGetProperty(
			m_algHandle,
			BCRYPT_OBJECT_LENGTH,
			reinterpret_cast<PBYTE>(&cbKeyObject),
			sizeof(DWORD),
			&cbData,
			0
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("Failed to set AES key length", status);

		return cbKeyObject;
	}

	DWORD AesEncryption::GetBlockByteLength() const
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");

		DWORD cbKeyObject = 0;
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
		const NTSTATUS status = BCryptGetProperty(
			m_algHandle,
			BCRYPT_BLOCK_LENGTH,
			reinterpret_cast<PBYTE>(&cbKeyObject),
			sizeof(DWORD),
			&cbData,
			0
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("Failed to set AES key length", status);

		return cbKeyObject;
	}

	void AesEncryption::SetChainingMode(const ChainingMode cm)
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");
		
		const std::wstring& mode = ChainingModeString.at(cm);
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptsetproperty
		const NTSTATUS status = BCryptSetProperty(
			m_algHandle,
			BCRYPT_CHAINING_MODE,
			reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(&mode[0])),
			static_cast<ULONG>(mode.size() * sizeof(wchar_t)),
			0
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("Failed to set chaining mode", status);
		m_chainingMode = cm;
	}

	CryptoKey AesEncryption::GenerateSymmetricKey(const std::vector<std::byte>& rgbAES128Key)
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");
		if (rgbAES128Key.empty())
			throw Error::Boring32Error("rgbAES128Key is empty");

		BCRYPT_KEY_HANDLE hKey = nullptr;
		DWORD cbKeyObject = GetObjectByteSize();
		std::vector<std::byte> keyObject(cbKeyObject, static_cast<std::byte>(0));
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgeneratesymmetrickey
		const NTSTATUS status = BCryptGenerateSymmetricKey(
			m_algHandle,
			&hKey,
			reinterpret_cast<PUCHAR>(&keyObject[0]),
			cbKeyObject,
			reinterpret_cast<PBYTE>(const_cast<std::byte*>(&rgbAES128Key[0])),
			(ULONG)rgbAES128Key.size(),
			0
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("Failed to set chaining mode", status);
		
		return CryptoKey(hKey, std::move(keyObject));
	}

	std::vector<std::byte> AesEncryption::Encrypt(
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::wstring_view string
	)
	{
		const std::byte* buffer = reinterpret_cast<const std::byte*>(&string[0]);
		return Encrypt(
			key,
			iv,
			std::vector<std::byte>(buffer, buffer + string.size() * sizeof(wchar_t))
		);
	}

	std::vector<std::byte> AesEncryption::Encrypt(
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText
	)
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");
		if (!key.GetHandle())
			throw Error::Boring32Error("Key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			if (iv.size() != GetBlockByteLength())
				throw Error::Boring32Error("IV must be the same size as the AES block lenth");
			pIV = reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<ULONG>(iv.size());
		}

		// Determine the byte size of the encrypted data
		DWORD cbData = 0;
		const DWORD flags = GetEncryptDecryptFlags();
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		NTSTATUS status = BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("BCryptEncrypt() failed to count bytes", status);

		// Actually do the encryption
		std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
		status = BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<PUCHAR>(&cypherText[0]),
			static_cast<ULONG>(cypherText.size()),
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("BCryptEncrypt() failed to encrypt", status);

		return cypherText;
	}

	std::vector<std::byte> AesEncryption::Decrypt(
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText
	)
	{
		if (!m_algHandle)
			throw Error::Boring32Error("Cipher algorithm not initialised");
		if (!key.GetHandle())
			throw Error::Boring32Error("Key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			if (iv.size() != GetBlockByteLength())
				throw Error::Boring32Error("IV must be the same size as the AES block lenth");
			pIV = reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<ULONG>(iv.size());
		}

		// Determine the byte size of the decrypted data
		DWORD cbData = 0;
		const DWORD flags = GetEncryptDecryptFlags();
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		NTSTATUS status = BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("BCryptDecrypt() failed to count bytes", status);

		// Actually do the decryption
		std::vector<std::byte> plainText(cbData, std::byte{ 0 });
		status = BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<PUCHAR>(&plainText[0]),
			static_cast<ULONG>(plainText.size()),
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NtStatusError("BCryptDecrypt() failed to decrypt", status);

		plainText.resize(cbData);
		return plainText;
	}

	DWORD AesEncryption::GetEncryptDecryptFlags() const
	{
		// BCRYPT_BLOCK_PADDING must not be used with the authenticated encryption modes(AES - CCM and AES - GCM)
		if (m_chainingMode == ChainingMode::NotSet)
			throw Error::Boring32Error("m_chainingMode is not set");
		if (m_chainingMode == ChainingMode::GaloisCounterMode)
			return 0;
		if (m_chainingMode == ChainingMode::CbcMac)
			return 0;
		return BCRYPT_BLOCK_PADDING;
	}
}
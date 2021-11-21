#include "pch.hpp"
//#include <ntstatus.h>
#include "include/Crypto/AesEncryption.hpp"

import boring32.error.ntstatuserror;

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
			throw std::runtime_error(__FUNCSIG__ ": m_chainingMode is not set");
		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
		NTSTATUS status = BCryptOpenAlgorithmProvider(
			&m_algHandle,
			BCRYPT_AES_ALGORITHM, // https://docs.microsoft.com/en-us/windows/win32/seccng/cng-algorithm-identifiers
			nullptr,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to create AES algorithm", status);
		SetChainingMode(m_chainingMode);
	}

	BCRYPT_ALG_HANDLE AesEncryption::GetHandle() const noexcept
	{
		return m_algHandle;
	}

	DWORD AesEncryption::GetObjectByteSize() const
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");

		DWORD cbKeyObject = 0;
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
		const NTSTATUS status = BCryptGetProperty(
			m_algHandle,
			BCRYPT_OBJECT_LENGTH,
			(PBYTE)&cbKeyObject,
			sizeof(DWORD),
			&cbData,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set AES key length", status);

		return cbKeyObject;
	}

	DWORD AesEncryption::GetBlockByteLength() const
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");

		DWORD cbKeyObject = 0;
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
		const NTSTATUS status = BCryptGetProperty(
			m_algHandle,
			BCRYPT_BLOCK_LENGTH,
			(PBYTE)&cbKeyObject,
			sizeof(DWORD),
			&cbData,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set AES key length", status);

		return cbKeyObject;
	}

	void AesEncryption::SetChainingMode(const ChainingMode cm)
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");
		
		const std::wstring& mode = ChainingModeString.at(cm);
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptsetproperty
		const NTSTATUS status = BCryptSetProperty(
			m_algHandle,
			BCRYPT_CHAINING_MODE,
			(PUCHAR)&mode[0],
			(ULONG)mode.size() * sizeof(wchar_t),
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set chaining mode", status);
		m_chainingMode = cm;
	}

	CryptoKey AesEncryption::GenerateSymmetricKey(const std::vector<std::byte>& rgbAES128Key)
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");
		if (rgbAES128Key.empty())
			throw std::invalid_argument(__FUNCSIG__ ": rgbAES128Key is empty");

		BCRYPT_KEY_HANDLE hKey = nullptr;
		DWORD cbKeyObject = GetObjectByteSize();
		std::vector<std::byte> keyObject(cbKeyObject, static_cast<std::byte>(0));
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgeneratesymmetrickey
		const NTSTATUS status = BCryptGenerateSymmetricKey(
			m_algHandle,
			&hKey,
			(PUCHAR)&keyObject[0],
			cbKeyObject,
			(PBYTE)&rgbAES128Key[0],
			(ULONG)rgbAES128Key.size(),
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set chaining mode", status);
		
		return CryptoKey(hKey, std::move(keyObject));
	}

	std::vector<std::byte> AesEncryption::Encrypt(
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::wstring& string
	)
	{
		const std::byte* buffer = (std::byte*)&string[0];
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
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");
		if (key.GetHandle() == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			if (iv.size() != GetBlockByteLength())
				throw std::invalid_argument(__FUNCSIG__ ": IV must be the same size as the AES block lenth");
			pIV = (PUCHAR)&iv[0];
			ivSize = (ULONG)iv.size();
		}

		// Determine the byte size of the encrypted data
		DWORD cbData = 0;
		const DWORD flags = GetEncryptDecryptFlags();
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		NTSTATUS status = BCryptEncrypt(
			key.GetHandle(),
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptEncrypt() failed to count bytes", status);

		// Actually do the encryption
		std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
		status = BCryptEncrypt(
			key.GetHandle(),
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			nullptr,
			pIV,
			ivSize,
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptEncrypt() failed to encrypt", status);

		return cypherText;
	}

	std::vector<std::byte> AesEncryption::Decrypt(
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText
	)
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");
		if (key.GetHandle() == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			if (iv.size() != GetBlockByteLength())
				throw std::invalid_argument(__FUNCSIG__ ": IV must be the same size as the AES block lenth");
			pIV = (PUCHAR)&iv[0];
			ivSize = (ULONG)iv.size();
		}

		// Determine the byte size of the decrypted data
		DWORD cbData = 0;
		const DWORD flags = GetEncryptDecryptFlags();
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		NTSTATUS status = BCryptDecrypt(
			key.GetHandle(),
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptDecrypt() failed to count bytes", status);

		// Actually do the decryption
		std::vector<std::byte> plainText(cbData, std::byte{ 0 });
		status = BCryptDecrypt(
			key.GetHandle(),
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			nullptr,
			pIV,
			ivSize,
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptDecrypt() failed to decrypt", status);

		plainText.resize(cbData);
		return plainText;
	}

	DWORD AesEncryption::GetEncryptDecryptFlags() const
	{
		// BCRYPT_BLOCK_PADDING must not be used with the authenticated encryption modes(AES - CCM and AES - GCM)
		if (m_chainingMode == ChainingMode::NotSet)
			throw std::runtime_error(__FUNCSIG__ ": m_chainingMode is not set");
		if (m_chainingMode == ChainingMode::GaloisCounterMode)
			return 0;
		if (m_chainingMode == ChainingMode::CbcMac)
			return 0;
		return BCRYPT_BLOCK_PADDING;
	}
}
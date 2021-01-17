#include "pch.hpp"
#include "include/Error/NtStatusError.hpp"
//#include <ntstatus.h>
#include "include/Crypto/AesEncryption.hpp"

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
namespace Boring32::Crypto
{
	AesEncryption::~AesEncryption()
	{
		if (m_algHandle)
		{
			//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptclosealgorithmprovider
			BCryptCloseAlgorithmProvider(m_algHandle, 0);
			m_algHandle = nullptr;
		}
	}

	AesEncryption::AesEncryption()
	:	m_algHandle(nullptr)
	{
		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
		NTSTATUS status = BCryptOpenAlgorithmProvider(
			&m_algHandle,
			BCRYPT_AES_ALGORITHM, // https://docs.microsoft.com/en-us/windows/win32/seccng/cng-algorithm-identifiers
			nullptr,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to create AES algorithm", status);
	}

	BCRYPT_ALG_HANDLE AesEncryption::GetHandle() const noexcept
	{
		return m_algHandle;
	}

	DWORD AesEncryption::GetObjectByteSize()
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

	void AesEncryption::SetChainingMode(const std::wstring& mode)
	{
		if (m_algHandle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": cipher algorithm not initialised");
		if (mode.empty())
			throw std::invalid_argument(__FUNCSIG__ ": mode is not specified");
		if (mode == BCRYPT_CHAIN_MODE_NA)
			throw std::invalid_argument(__FUNCSIG__ ": AES requires a chaining mode");

		const NTSTATUS status = BCryptSetProperty(
			m_algHandle,
			BCRYPT_CHAINING_MODE,
			(PUCHAR)&mode[0],
			(ULONG)mode.size()*sizeof(wchar_t),
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set chaining mode", status);
	}

	CryptoKey AesEncryption::GenerateSymmetricKey(const std::vector<std::byte>& rgbAES128Key)
	{
		BCRYPT_KEY_HANDLE hKey = nullptr;
		DWORD cbKeyObject = GetObjectByteSize();
		std::vector<std::byte> keyObject(cbKeyObject, static_cast<std::byte>(0));
		const NTSTATUS status = BCryptGenerateSymmetricKey(
			m_algHandle,
			&hKey,
			reinterpret_cast<PUCHAR>(&keyObject[0]),
			cbKeyObject,
			(PBYTE)&rgbAES128Key[0],
			static_cast<ULONG>(rgbAES128Key.size()),
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to set chaining mode", status);
		
		//BCryptDestroyKey(hKey);
		return CryptoKey(hKey);
	}
}
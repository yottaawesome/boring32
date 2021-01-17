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
		DWORD cbKeyObject = 0;
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
		NTSTATUS status = BCryptGetProperty(
			m_algHandle,
			BCRYPT_OBJECT_LENGTH,
			(PBYTE)&cbKeyObject,
			sizeof(DWORD),
			&cbData,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to AES key length", status);

		return cbKeyObject;
	}
}
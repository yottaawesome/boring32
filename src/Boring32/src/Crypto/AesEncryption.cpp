#include "pch.hpp"
#include "include/Error/NtStatusError.hpp"
//#include <ntstatus.h>
#include "include/Crypto/AesEncryption.hpp"

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
			BCRYPT_AES_ALGORITHM,
			NULL,
			0
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": failed to create AES algorithm", status);
	}

	BCRYPT_ALG_HANDLE AesEncryption::GetHandle() const noexcept
	{
		return m_algHandle;
	}
}
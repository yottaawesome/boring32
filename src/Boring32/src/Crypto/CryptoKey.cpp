#include "pch.hpp"
#include "include/Error/NtStatusError.hpp"
//#include <ntstatus.h>
#include "include/Crypto/CryptoKey.hpp"

// See: https://docs.microsoft.com/en-us/windows/win32/seccng/encrypting-data-with-cng
namespace Boring32::Crypto
{
	std::shared_ptr<void> Generate(BCRYPT_KEY_HANDLE ptr)
	{
		return std::shared_ptr<void>{
			ptr,
			[](BCRYPT_KEY_HANDLE ptr) { BCryptDestroyKey(ptr); }
		};
	}

	void CryptoKey::Close()
	{
		if (m_keyHandle)
		{
			BCryptDestroyKey(m_keyHandle);
			m_keyHandle = nullptr;
		}
	}

	CryptoKey::~CryptoKey()
	{
		Close();
	}

	CryptoKey::CryptoKey()
	:	m_keyHandle(nullptr)
	{ }

	CryptoKey::CryptoKey(BCRYPT_KEY_HANDLE const keyHandle)
		: m_keyHandle(keyHandle)
	{ }

	CryptoKey::CryptoKey(CryptoKey&& other) noexcept
	:	m_keyHandle(nullptr)
	{
		Move(other);
	}
	
	CryptoKey& CryptoKey::operator=(CryptoKey&& other) noexcept
	{
		return Move(other);
	}
	
	CryptoKey& CryptoKey::Move(CryptoKey& other) noexcept
	{
		Close();
		m_keyHandle = other.m_keyHandle;
		m_keyHandle = nullptr;
		return *this;
	}

	BCRYPT_KEY_HANDLE CryptoKey::GetHandle() const noexcept
	{
		return m_keyHandle;
	}
}
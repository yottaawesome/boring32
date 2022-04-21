module;

#include "pch.hpp"
#include <memory>
#include <vector>
//#include <ntstatus.h>

module boring32.crypto:cryptokey;

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

	CryptoKey::CryptoKey(BCRYPT_KEY_HANDLE const keyHandle, std::vector<std::byte>&& keyObject)
	:	m_keyHandle(keyHandle),
		m_keyObject(std::move(keyObject))
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
		other.m_keyHandle = nullptr;
		m_keyObject = std::move(other.m_keyObject);
		return *this;
	}

	BCRYPT_KEY_HANDLE CryptoKey::GetHandle() const noexcept
	{
		return m_keyHandle;
	}
}
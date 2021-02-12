#include "pch.hpp"
#include <algorithm>
#include <stdexcept>
#include <dpapi.h>
#include "include/Error/Win32Error.hpp"
#include "include/Crypto/SecureString.hpp"

namespace Boring32::Crypto
{
	SecureString::~SecureString()
	{
		Clear();
	}
	
	SecureString::SecureString()
	:	m_characters(0),
		m_encryptionType(EncryptionType::SameProcess)
	{ }

	SecureString::SecureString(const std::wstring& value)
	:	m_characters(0),
		m_encryptionType(EncryptionType::SameProcess)
	{
		SetValue(value);
	}

	SecureString::SecureString(const std::wstring& value, const EncryptionType encryptionType)
	:	m_characters(0),
		m_encryptionType(encryptionType)
	{
		SetValue(value);
	}

	void SecureString::SetValue(const std::wstring& value)
	{
		Clear();

		// We need the character count when the client asks to
		// decrypt. Strictly speaking, keeping this data
		// violates Shannon's perfect secrecy rule on encrypted 
		// data, but we can live with it
		m_characters = (DWORD)value.size();
		
		// Figure out if we need to align to the encryption block size
		const DWORD bytesOfPlainData = (DWORD) value.size() * sizeof(wchar_t);
		const DWORD bytesModBlock = bytesOfPlainData % CRYPTPROTECTMEMORY_BLOCK_SIZE;
		const DWORD bytesOfEncryptedData = bytesModBlock
			? bytesOfPlainData + CRYPTPROTECTMEMORY_BLOCK_SIZE - bytesModBlock
			: bytesOfPlainData;

		// Copy our plain data across to the buffer that will be
		// encrypted in-place
		const std::byte* buffer = (std::byte*)&value[0];
		m_encryptedData = std::vector<std::byte>(buffer, buffer+bytesOfPlainData);
		
		// Resize to a multiple of the encryption block size if
		// we need to
		if (bytesOfEncryptedData > bytesOfPlainData)
			m_encryptedData.resize(bytesOfEncryptedData);

		// Actually perform the encryption now
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectmemory
		const bool succeeded = CryptProtectMemory(
			&m_encryptedData[0],
			(DWORD)m_encryptedData.size(),
			(DWORD)m_encryptionType
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptProtectMemory() failed", GetLastError());
	}

	void SecureString::GetValue(std::wstring& value)
	{
		if (m_encryptedData.empty())
			throw std::runtime_error(__FUNCSIG__ ": nothing to decrypt");

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectmemory
		// De-encrypt
		bool succeeded = CryptUnprotectMemory(
			&m_encryptedData[0],
			(DWORD)m_encryptedData.size(),
			(DWORD)m_encryptionType
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptUnprotectMemory() failed", GetLastError());

		// Set the out parameter
		value = std::wstring((wchar_t*)&m_encryptedData[0], m_characters);

		// Re-encrypt
		succeeded = CryptProtectMemory(
			&m_encryptedData[0],
			(DWORD)m_encryptedData.size(),
			(DWORD)m_encryptionType
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptProtectMemory() failed", GetLastError());
	}

	void SecureString::Clear()
	{
		std::fill(m_encryptedData.begin(), m_encryptedData.end(), (std::byte)0);
		m_encryptedData.clear();
		m_characters = 0;
	}

	bool SecureString::HasData() const noexcept
	{
		return m_encryptedData.empty() == false;
	}
}
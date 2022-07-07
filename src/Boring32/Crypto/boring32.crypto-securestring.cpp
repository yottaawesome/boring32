module;

#include <algorithm>
#include <string>
#include <source_location>
#include <Windows.h>
#include <dpapi.h>

module boring32.crypto:securestring;
import boring32.error;

namespace Boring32::Crypto
{
	SecureString::~SecureString()
	{
		Clear();
	}
	
	SecureString::SecureString()
	:	m_characters(0),
		m_encryptionType(EncryptionType::SameProcess),
		m_isEncrypted(false)
	{ }

	SecureString::SecureString(const std::wstring& value)
	:	m_characters(0),
		m_encryptionType(EncryptionType::SameProcess),
		m_isEncrypted(false)
	{
		SetValueAndEncrypt(value);
	}

	SecureString::SecureString(const std::wstring& value, const EncryptionType encryptionType)
	:	m_characters(0),
		m_encryptionType(encryptionType),
		m_isEncrypted(false)
	{
		SetValueAndEncrypt(value);
	}

	SecureString::operator bool() const noexcept
	{
		return m_isEncrypted;
	}
	
	SecureString::operator std::wstring()
	{
		std::wstring out;
		DecryptAndCopy(out);
		return out;
	}

	SecureString& SecureString::operator=(const std::wstring& newValue)
	{
		SetValueAndEncrypt(newValue);
		return *this;
	}

	void SecureString::SetValueAndEncrypt(const std::wstring& value)
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
		m_protectedString = value;
		
		// Resize to a multiple of the encryption block size if
		// we need to
		if (bytesOfEncryptedData > bytesOfPlainData)
			m_protectedString.resize(bytesOfEncryptedData / sizeof(wchar_t));

		// Actually perform the encryption now
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectmemory
		Encrypt();
	}

	void SecureString::DecryptAndCopy(std::wstring& value)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectmemory
		// De-encrypt
		Decrypt();

		// Set the out parameter
		value = m_protectedString;
		value.resize(m_characters);

		// Re-encrypt
		Encrypt();
	}

	const std::wstring& SecureString::GetValue() const
	{
		return m_protectedString;
	}

	void SecureString::Clear()
	{
		std::fill(m_protectedString.begin(), m_protectedString.end(), '\0');
		m_protectedString.clear();
		m_characters = 0;
		m_isEncrypted = false;
	}

	bool SecureString::HasData() const noexcept
	{
		return m_protectedString.empty() == false;
	}

	void SecureString::Encrypt()
	{
		if (m_isEncrypted)
			return;
		if (m_protectedString.empty())
			throw Error::Boring32Error("Nothing to decrypt");

		const bool succeeded = CryptProtectMemory(
			(void*)&m_protectedString[0],
			(DWORD)m_protectedString.size() * sizeof(wchar_t),
			(DWORD)m_encryptionType
		);
		if (succeeded == false)
			throw Error::Win32Error("CryptProtectMemory() failed", GetLastError());
		m_isEncrypted = true;
	}

	void SecureString::Decrypt()
	{
		if (m_isEncrypted == false)
			return;
		if (m_protectedString.empty())
			throw Error::Boring32Error("Nothing to decrypt");

		const bool succeeded = CryptUnprotectMemory(
			(void*)&m_protectedString[0],
			(DWORD)m_protectedString.size() * sizeof(wchar_t),
			(DWORD)m_encryptionType
		);
		if (succeeded == false)
			throw Error::Win32Error("CryptUnprotectMemory() failed", GetLastError());
		m_isEncrypted = false;
	}

	bool SecureString::IsCurrentlyEncrypted() const noexcept
	{
		return m_isEncrypted;
	}
}
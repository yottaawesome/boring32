#include "pch.hpp"
#include <Windows.h>
#include <Wincrypt.h>
#include "include/Error/Win32Error.hpp"
#include "include/Crypto/CryptoFuncs.hpp"

namespace Boring32::Crypto
{
	// See also a complete example on MSDN at:
	// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-using-cryptprotectdata

	std::vector<std::byte> EncryptString(
		const std::wstring& str,
		const std::wstring& password,
		const std::wstring& description
	)
	{
		DATA_BLOB dataIn;
		dataIn.pbData = (BYTE*)&str[0];
		dataIn.cbData = (DWORD)str.size()*sizeof(wchar_t);

		DATA_BLOB additionalEntropy{ 0 };
		if (password.empty() == false)
		{
			additionalEntropy.pbData = (BYTE*)&password[0];
			additionalEntropy.cbData = (DWORD)password.size() * sizeof(wchar_t);
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
		DATA_BLOB encryptedBlob{ 0 };
		const bool succeeded = CryptProtectData(
			&dataIn,					// The data to encrypt.
			description.empty()			// An optional description string.
				? nullptr 
				: description.c_str(),	
			password.empty()			// Optional additional entropy to
				? nullptr				// to encrypt the string with, e.g.
				: &additionalEntropy,	// a password.
			nullptr,					// Reserved.
			nullptr,					// Pass a PromptStruct.
			0,							// Flags.
			&encryptedBlob				// Receives the encrypted information.
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptProtectData() failed", GetLastError());

		// Should we really return std::byte instead of Windows' BYTE?
		// Using std::byte means we'll need to cast at the API call.
		std::vector<std::byte> returnValue(
			(std::byte*)encryptedBlob.pbData, 
			(std::byte*)encryptedBlob.pbData + encryptedBlob.cbData
		);
		if (encryptedBlob.pbData)
			LocalFree(encryptedBlob.pbData);

		return returnValue;
	}

	std::wstring DecryptString(
		const std::vector<std::byte>& encryptedData,
		const std::wstring& password,
		std::wstring& outDescription
	)
	{
		DATA_BLOB encryptedBlob;
		encryptedBlob.pbData = (BYTE*)&encryptedData[0];
		encryptedBlob.cbData = (DWORD)encryptedData.size();

		DATA_BLOB additionalEntropy{ 0 };
		if (password.empty() == false)
		{
			additionalEntropy.pbData = (BYTE*)&password[0];
			additionalEntropy.cbData = (DWORD)password.size() * sizeof(wchar_t);
		}

		DATA_BLOB decryptedBlob;
		LPWSTR pDescrOut = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
		const bool succeeded = CryptUnprotectData(
			&encryptedBlob,				// the encrypted data
			&pDescrOut,					// Optional description
			password.empty()			// Optional additional entropy
				? nullptr				// used to encrypt the string 
				: &additionalEntropy,	// with, e.g. a password
			nullptr,					// Reserved
			nullptr,					// Optional prompt structure
			0,							// Flags
			&decryptedBlob				// Receives the decrypted data
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptUnprotectData() failed", GetLastError());

		if (pDescrOut)
		{
			outDescription = pDescrOut;
			LocalFree(pDescrOut);
		}

		std::wstring returnValue(
			(wchar_t*)decryptedBlob.pbData, 
			decryptedBlob.cbData / sizeof(wchar_t)
		);
		if (decryptedBlob.pbData)
			LocalFree(decryptedBlob.pbData);

		return returnValue;
	}
}
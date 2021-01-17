#include "pch.hpp"
#include <Windows.h>
#include <Wincrypt.h>
#include "include/Error/Win32Error.hpp"
#include "include/Crypto/CryptoFuncs.hpp"

namespace Boring32::Crypto
{
	// See also a complete example on MSDN at:
	// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-using-cryptprotectdata

	std::vector<std::byte> Encrypt(
		const std::vector<std::byte>& data,
		const std::wstring& password,
		const std::wstring& description
	)
	{
		DATA_BLOB dataIn;
		dataIn.pbData = (BYTE*)&data[0];
		dataIn.cbData = (DWORD)data.size();

		DATA_BLOB additionalEntropy{ 0 };
		if (password.empty() == false)
		{
			additionalEntropy.pbData = (BYTE*)&password[0];
			additionalEntropy.cbData = (DWORD)password.size()*sizeof(wchar_t);
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
		DATA_BLOB encryptedBlob{ 0 };
		const wchar_t* descriptionCStr = description.empty()
			? nullptr
			: description.c_str();
		DATA_BLOB* const entropy = password.empty()
			? nullptr
			: &additionalEntropy;
		const bool succeeded = CryptProtectData(
			&dataIn,					// The data to encrypt.
			descriptionCStr,			// An optional description string.
			entropy,					// Optional additional entropy to
										// to encrypt the string with, e.g.
										// a password.
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

	std::vector<std::byte> Encrypt(
		const std::wstring& str,
		const std::wstring& password,
		const std::wstring& description
	)
	{
		const std::byte* buffer = (std::byte*)&str[0];
		return Encrypt(
			std::vector<std::byte>(buffer, buffer + str.size() * sizeof(wchar_t)),
			password, 
			description
		);
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
		LPWSTR descrOut = nullptr;
		DATA_BLOB* const entropy = password.empty()
			? nullptr
			: &additionalEntropy;
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
		const bool succeeded = CryptUnprotectData(
			&encryptedBlob,				// the encrypted data
			&descrOut,					// Optional description
			entropy,					// Optional additional entropy
										// used to encrypt the string
										// with, e.g. a password
			nullptr,					// Reserved
			nullptr,					// Optional prompt structure
			0,							// Flags
			&decryptedBlob				// Receives the decrypted data
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptUnprotectData() failed", GetLastError());

		if (descrOut)
		{
			outDescription = descrOut;
			LocalFree(descrOut);
		}

		std::wstring returnValue(
			reinterpret_cast<wchar_t*>(decryptedBlob.pbData), 
			decryptedBlob.cbData / sizeof(wchar_t)
		);
		if (decryptedBlob.pbData)
			LocalFree(decryptedBlob.pbData);

		return returnValue;
	}
}
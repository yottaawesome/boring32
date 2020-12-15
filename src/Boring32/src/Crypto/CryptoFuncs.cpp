#include "pch.hpp"
#include <Windows.h>
#include <Wincrypt.h>
#include "include/Error/Win32Error.hpp"
#include "include/Crypto/CryptoFuncs.hpp"

namespace Boring32::Crypto
{
	std::vector<std::byte> EncryptString(
		const std::wstring& description,
		const std::wstring& str
	)
	{
		DATA_BLOB dataIn;
		dataIn.pbData = (BYTE*)&str[0];
		dataIn.cbData = (DWORD)str.size()*sizeof(wchar_t);

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
		DATA_BLOB dataOut;
		const bool succeeded = CryptProtectData(
			&dataIn,					// The data to encrypt
			description.empty() 
				? nullptr 
				: description.c_str(),	// A description string. 
			nullptr,					// Optional additional entropy
			nullptr,					// Reserved.
			nullptr,					// Pass a PromptStruct.
			0,							// Flags
			&dataOut					// Receives the encrypted information
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptProtectData() failed", GetLastError());

		// Should we really return std::byte instead of Windows' BYTE?
		// Using std::byte means we'll need to cast at the API call.
		std::vector<std::byte> returnValue(
			(std::byte*)dataOut.pbData, 
			(std::byte*)dataOut.pbData + dataOut.cbData
		);
		if (dataOut.pbData)
			LocalFree(dataOut.pbData);

		return returnValue;
	}

	std::wstring DecryptString(
		const std::vector<std::byte>& encryptedData,
		std::wstring& outDescription
	)
	{
		DATA_BLOB encryptedBlob;
		encryptedBlob.pbData = (BYTE*)&encryptedData[0];
		encryptedBlob.cbData = (DWORD)encryptedData.size();

		DATA_BLOB decryptedBlob;
		LPWSTR pDescrOut = nullptr;
		const bool succeeded = CryptUnprotectData(
			&encryptedBlob,		// the encrypted data
			&pDescrOut,			// optional description
			nullptr,            // Optional entropy
			nullptr,            // Reserved
			nullptr,            // optional prompt structure
			0,					// flags
			&decryptedBlob		// receives the decrypted data
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
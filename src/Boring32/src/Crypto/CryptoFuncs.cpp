#include "pch.hpp"
#include <Windows.h>
#include <Wincrypt.h>
#include "include/Error/Win32Error.hpp"
#include "include/Error/NtStatusError.hpp"
#include "include/Crypto/CryptoFuncs.hpp"
#include "include/Crypto/CryptoKey.hpp"

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

	std::vector<std::byte> Encrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText,
		const DWORD flags
	)
	{
		if (key.GetHandle() == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			if (iv.size() != blockByteLength)
				throw std::invalid_argument(__FUNCSIG__ ": IV must be the same size as the AES block lenth");
			pIV = (PUCHAR)&iv[0];
			ivSize = (ULONG)iv.size();
		}

		// Determine the byte size of the encrypted data
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		NTSTATUS status = BCryptEncrypt(
			key.GetHandle(),
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptEncrypt() failed to count bytes", status);

		// Actually do the encryption
		std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
		status = BCryptEncrypt(
			key.GetHandle(),
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			nullptr,
			pIV,
			ivSize,
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptEncrypt() failed to encrypt", status);

		return cypherText;
	}

	std::vector<std::byte> Decrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText,
		const DWORD flags
	)
	{
		if (key.GetHandle() == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (iv.empty() == false)
		{
			// Do all cipher algs require this?
			if (iv.size() != blockByteLength)
				throw std::invalid_argument(__FUNCSIG__ ": IV must be the same size as the AES block lenth");
			pIV = (PUCHAR)&iv[0];
			ivSize = (ULONG)iv.size();
		}

		// Determine the byte size of the decrypted data
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		NTSTATUS status = BCryptDecrypt(
			key.GetHandle(),
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptDecrypt() failed to count bytes", status);

		// Actually do the decryption
		std::vector<std::byte> plainText(cbData, std::byte{ 0 });
		status = BCryptDecrypt(
			key.GetHandle(),
			(PUCHAR)&cypherText[0],
			(ULONG)cypherText.size(),
			nullptr,
			pIV,
			ivSize,
			(PUCHAR)&plainText[0],
			(ULONG)plainText.size(),
			&cbData,
			flags
		);
		if (BCRYPT_SUCCESS(status) == false)
			throw Error::NtStatusError(__FUNCSIG__ ": BCryptDecrypt() failed to decrypt", status);

		plainText.resize(cbData);
		return plainText;
	}

	std::string ToBase64String(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		DWORD size = 0;
		bool succeeded = CryptBinaryToStringA(
			(BYTE*)&bytes[0],
			(DWORD)bytes.size(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			nullptr,
			&size
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptBinaryToStringA() failed when calculating size");
		if (size == 0)
			return "";

		std::string returnVal(size, L'\0');
		succeeded = CryptBinaryToStringA(
			(BYTE*)&bytes[0],
			(DWORD)bytes.size(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			(LPSTR)&returnVal[0],
			&size
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptBinaryToStringA() failed when encoding");
		// Remove terminating null character
		if (returnVal.empty() == false)
			returnVal.pop_back();

		return returnVal;
	}

	std::wstring ToBase64WString(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		DWORD size = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-cryptbinarytostringw
		bool succeeded = CryptBinaryToStringW(
			(BYTE*)&bytes[0],
			(DWORD)bytes.size(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			nullptr,
			&size
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptBinaryToStringW() failed when calculating size");
		if (size == 0)
			return L"";

		std::wstring returnVal(size, L'\0');
		succeeded = CryptBinaryToStringW(
			(BYTE*)&bytes[0],
			(DWORD)bytes.size(),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			(LPWSTR)&returnVal[0],
			&size
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptBinaryToStringW() failed when encoding");
		// Remove terminating null character
		if (returnVal.empty() == false)
			returnVal.pop_back();

		return returnVal;
	}

	std::vector<std::byte> ToBinary(const std::wstring& base64)
	{
		DWORD byteSize = 0;
		BOOL succeeded = CryptStringToBinaryW(
			&base64[0],
			0,
			CRYPT_STRING_BASE64,
			nullptr,
			&byteSize,
			nullptr,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptStringToBinaryW() failed when encoding");

		std::vector<std::byte> returnVal(byteSize);
		succeeded = CryptStringToBinaryW(
			&base64[0],
			0,
			CRYPT_STRING_BASE64,
			(BYTE*)&returnVal[0],
			&byteSize,
			nullptr,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CryptStringToBinaryW() failed when encoding");
		returnVal.resize(byteSize);
		return returnVal;
	}
}
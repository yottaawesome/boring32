module;

#include <vector>
#include <stdexcept>
#include <source_location>
#include <string>
#include <Windows.h>
#include <dpapi.h> // not including this header causes symbol has already been defined error
#include <Wincrypt.h>

module boring32.crypto:functions;
import boring32.error;

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
		dataIn.pbData = reinterpret_cast<BYTE*>(const_cast<std::byte*>(&data[0]));
		dataIn.cbData = static_cast<DWORD>(data.size());

		DATA_BLOB additionalEntropy{ 0 };
		if (password.empty() == false)
		{
			additionalEntropy.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<DWORD>(password.size()*sizeof(wchar_t));
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
			throw Error::Win32Error(std::source_location::current(), "CryptProtectData() failed", GetLastError());

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
			throw Error::Win32Error(std::source_location::current(), "CryptUnprotectData() failed", GetLastError());

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
			throw Error::NtStatusError(std::source_location::current(), "BCryptEncrypt() failed to count bytes", status);

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
			throw Error::NtStatusError(std::source_location::current(), "BCryptEncrypt() failed to encrypt", status);

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
			throw Error::NtStatusError(std::source_location::current(), "BCryptDecrypt() failed to count bytes", status);

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
			throw Error::NtStatusError(std::source_location::current(), "BCryptDecrypt() failed to decrypt", status);

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
			throw Error::Win32Error(std::source_location::current(), "CryptBinaryToStringA() failed when calculating size");
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
			throw Error::Win32Error(std::source_location::current(), "CryptBinaryToStringA() failed when encoding");
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
			throw Error::Win32Error(std::source_location::current(), "CryptBinaryToStringW() failed when calculating size");
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
			throw Error::Win32Error(std::source_location::current(), "CryptBinaryToStringW() failed when encoding");
		// Remove terminating null character
		if (returnVal.empty() == false)
			returnVal.pop_back();

		return returnVal;
	}

	std::vector<std::byte> ToBinary(const std::wstring& base64)
	{
		DWORD byteSize = 0;
		bool succeeded = CryptStringToBinaryW(
			&base64[0],
			0,
			CRYPT_STRING_BASE64,
			nullptr,
			&byteSize,
			nullptr,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(std::source_location::current(), "CryptStringToBinaryW() failed when calculating size");

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
			throw Error::Win32Error(std::source_location::current(), "CryptStringToBinaryW() failed when decoding");

		returnVal.resize(byteSize);
		return returnVal;
	}

	std::vector<std::byte> EncodeAsnString(const std::wstring& name)
	{
		DWORD encoded = 0;
		// CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG is required or the encoding
		// produces subtle differences in the encoded bytes (DC3 vs FF in 
		// original buffer), which causes the match to fail
		// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certstrtonamew
		const DWORD flags = CERT_X500_NAME_STR | CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG;
		bool succeeded = CertStrToNameW(
			X509_ASN_ENCODING,
			name.c_str(),
			flags,
			nullptr,
			nullptr,
			&encoded,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(std::source_location::current(), "CertStrToNameW() failed", GetLastError());

		std::vector<std::byte> byte(encoded);
		succeeded = CertStrToNameW(
			X509_ASN_ENCODING,
			name.c_str(),
			flags,
			nullptr,
			(BYTE*)&byte[0],
			&encoded,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(std::source_location::current(), "CertStrToNameW() failed", GetLastError());
		byte.resize(encoded);

		return byte;
	}

	std::wstring FormatAsnNameBlob(
		const CERT_NAME_BLOB& certName,
		const DWORD format
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certnametostrw
		DWORD characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			(CERT_NAME_BLOB*)&certName,
			format,
			nullptr,
			0
		);
		if (characterSize == 0)
			return L"";

		std::wstring name(characterSize, '\0');
		characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			(CERT_NAME_BLOB*)&certName,
			format,
			&name[0],
			(DWORD)name.size()
		);
		name.pop_back(); // remove excess null character

		return name;
	}
}
export module boring32:crypto_functions;
import boring32.shared;
import :error;
import :crypto_cryptokey;

export namespace Boring32::Crypto
{
	// See also a complete example on MSDN at:
	// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-using-cryptprotectdata

	std::vector<std::byte> Encrypt(
		const std::vector<std::byte>& data,
		const std::wstring& password,
		const std::wstring& description
	)
	{
		Win32::DATA_BLOB dataIn{
			.cbData = static_cast<Win32::DWORD>(data.size()),
			.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&data[0]))
		};

		Win32::DATA_BLOB additionalEntropy{ 0 };
		if (not password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<Win32::DWORD>(password.size() * sizeof(wchar_t));
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
		Win32::DATA_BLOB encryptedBlob{ 0 };
		const wchar_t* descriptionCStr = description.empty()
			? nullptr
			: description.c_str();
		Win32::DATA_BLOB* const entropy = password.empty()
			? nullptr
			: &additionalEntropy;
		bool succeeded = Win32::CryptProtectData(
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
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CryptProtectData() failed");

		// Should we really return std::byte instead of Windows' BYTE?
		// Using std::byte means we'll need to cast at the API call.
		std::vector<std::byte> returnValue(
			(std::byte*)encryptedBlob.pbData,
			(std::byte*)encryptedBlob.pbData + encryptedBlob.cbData
		);
		if (encryptedBlob.pbData)
			Win32::LocalFree(encryptedBlob.pbData);

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
		Win32::DATA_BLOB encryptedBlob;
		encryptedBlob.pbData = (Win32::BYTE*)&encryptedData[0];
		encryptedBlob.cbData = (Win32::DWORD)encryptedData.size();

		Win32::DATA_BLOB additionalEntropy{ 0 };
		if (not password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<Win32::DWORD>(password.size() * sizeof(wchar_t));
		}

		Win32::DATA_BLOB decryptedBlob;
		Win32::LPWSTR descrOut = nullptr;
		Win32::DATA_BLOB* const entropy = password.empty()
			? nullptr
			: &additionalEntropy;
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
		const bool succeeded = Win32::CryptUnprotectData(
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
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CryptUnprotectData() failed");

		if (descrOut)
		{
			outDescription = descrOut;
			Win32::LocalFree(descrOut);
		}

		std::wstring returnValue(
			reinterpret_cast<wchar_t*>(decryptedBlob.pbData),
			decryptedBlob.cbData / sizeof(wchar_t)
		);
		if (decryptedBlob.pbData)
			Win32::LocalFree(decryptedBlob.pbData);

		return returnValue;
	}

	std::vector<std::byte> Decrypt(
		const Win32::DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText,
		const Win32::DWORD flags
	)
	{
		if (not key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		Win32::PUCHAR pIV = nullptr;
		Win32::ULONG ivSize = 0;
		if (not iv.empty())
		{
			// Do all cipher algs require this?
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block length");
			pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<Win32::ULONG>(iv.size());
		}

		// Determine the byte size of the decrypted data
		Win32::DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		Win32::NTSTATUS status = Win32::BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<Win32::ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (not Win32::BCryptSuccess(status))
			throw Error::NTStatusError(status, "BCryptDecrypt() failed to count bytes");

		// Actually do the decryption
		std::vector<std::byte> plainText(cbData, std::byte{ 0 });
		status = Win32::BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<Win32::ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<Win32::PUCHAR>(&plainText[0]),
			static_cast<Win32::ULONG>(plainText.size()),
			&cbData,
			flags
		);
		if (not Win32::BCryptSuccess(status))
			throw Error::NTStatusError(status, "BCryptDecrypt() failed to decrypt");

		plainText.resize(cbData);
		return plainText;
	}

	std::vector<std::byte> Encrypt(
		Win32::DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText,
		Win32::DWORD flags
	)
	{
		if (not key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		Win32::PUCHAR pIV = nullptr;
		Win32::ULONG ivSize = 0;
		if (not iv.empty())
		{
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block lenth");
			pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<Win32::ULONG>(iv.size());
		}

		// Determine the byte size of the encrypted data
		Win32::DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		Win32::NTSTATUS status = Win32::BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<Win32::ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (not Win32::BCryptSuccess(status))
			throw Error::NTStatusError(status, "BCryptEncrypt() failed to count bytes");

		// Actually do the encryption
		std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
		status = Win32::BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<Win32::ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<Win32::PUCHAR>(&cypherText[0]),
			static_cast<Win32::ULONG>(cypherText.size()),
			&cbData,
			flags
		);
		if (not Win32::BCryptSuccess(status))
			throw Error::NTStatusError(status, "BCryptEncrypt() failed to encrypt");

		return cypherText;
	}

	std::string ToBase64String(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		Win32::DWORD size = 0;
		constexpr Win32::DWORD flags = Win32::CryptStringBase64 | Win32::CryptStringNoCrLf;
		bool succeeded = Win32::CryptBinaryToStringA(
			reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<Win32::DWORD>(bytes.size()),
			flags,
			nullptr,
			&size
		);
		if (not succeeded)
			throw Error::Win32Error("CryptBinaryToStringA() failed when calculating size");
		if (size == 0)
			return {};

		std::string returnVal(size, L'\0');
		succeeded = Win32::CryptBinaryToStringA(
			reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<Win32::DWORD>(bytes.size()),
			flags,
			static_cast<Win32::LPSTR>(&returnVal[0]),
			&size
		);
		if (not succeeded)
			throw Error::Win32Error("CryptBinaryToStringA() failed when encoding");
		// Remove terminating null character
		if (not returnVal.empty())
			returnVal.pop_back();

		return returnVal;
	}

	std::wstring ToBase64WString(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		Win32::DWORD size = 0;
		constexpr Win32::DWORD flags = Win32::CryptStringBase64 | Win32::CryptStringNoCrLf;
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-cryptbinarytostringw
		bool succeeded = Win32::CryptBinaryToStringW(
			reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<Win32::DWORD>(bytes.size()),
			flags,
			nullptr,
			&size
		);
		if (not succeeded)
			throw Error::Win32Error("CryptBinaryToStringW() failed when calculating size");
		if (size == 0)
			return {};

		std::wstring returnVal(size, L'\0');
		succeeded = Win32::CryptBinaryToStringW(
			reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<Win32::DWORD>(bytes.size()),
			flags,
			static_cast<Win32::LPWSTR>(&returnVal[0]),
			&size
		);
		if (not succeeded)
			throw Error::Win32Error("CryptBinaryToStringW() failed when encoding");
		// Remove terminating null character
		if (not returnVal.empty())
			returnVal.pop_back();

		return returnVal;
	}

	std::vector<std::byte> ToBinary(const std::wstring& base64)
	{
		Win32::DWORD byteSize = 0;
		bool succeeded = Win32::CryptStringToBinaryW(
			&base64[0],
			0,
			Win32::CryptStringBase64,
			nullptr,
			&byteSize,
			nullptr,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error("CryptStringToBinaryW() failed when calculating size");

		std::vector<std::byte> returnVal(byteSize);
		succeeded = Win32::CryptStringToBinaryW(
			&base64[0],
			0,
			Win32::CryptStringBase64,
			reinterpret_cast<Win32::BYTE*>(&returnVal[0]),
			&byteSize,
			nullptr,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error("CryptStringToBinaryW() failed when decoding");

		returnVal.resize(byteSize);
		return returnVal;
	}

	std::vector<std::byte> EncodeAsnString(const std::wstring& name)
	{
		Win32::DWORD encoded = 0;
		// CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG is required or the encoding
		// produces subtle differences in the encoded bytes (DC3 vs FF in 
		// original buffer), which causes the match to fail
		// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certstrtonamew
		const Win32::DWORD flags = Win32::CertX500NameStr | Win32::CertNameStrForceUtf8DirStrFlag;
		bool succeeded = Win32::CertStrToNameW(
			Win32::X509AsnEncoding,
			name.c_str(),
			flags,
			nullptr,
			nullptr,
			&encoded,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CertStrToNameW() failed");

		std::vector<std::byte> bytes(encoded);
		succeeded = Win32::CertStrToNameW(
			Win32::X509AsnEncoding,
			name.c_str(),
			flags,
			nullptr,
			reinterpret_cast<Win32::BYTE*>(&bytes[0]),
			&encoded,
			nullptr
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CertStrToNameW() failed");

		bytes.resize(encoded);
		return bytes;
	}

	std::wstring FormatAsnNameBlob(const Win32::CERT_NAME_BLOB& certName, Win32::DWORD format)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certnametostrw
		Win32::DWORD characterSize = Win32::CertNameToStrW(
			Win32::X509AsnEncoding,
			const_cast<Win32::CERT_NAME_BLOB*>(&certName),
			format,
			nullptr,
			0
		);
		if (characterSize == 0)
			return L"";

		std::wstring name(characterSize, '\0');
		characterSize = Win32::CertNameToStrW(
			Win32::X509AsnEncoding,
			const_cast<Win32::CERT_NAME_BLOB*>(&certName),
			format,
			&name[0],
			static_cast<Win32::DWORD>(name.size())
		);
		name.pop_back(); // remove excess null character

		return name;
	}

	std::vector<Win32::PCCERT_CHAIN_CONTEXT> FindChainInStore(Win32::HCERTSTORE hCertStore, const std::wstring& issuer)
	{
		if (not hCertStore)
			throw Error::Boring32Error("hCertStore cannot be null");
		if (issuer.empty())
			throw Error::Boring32Error("issuer cannot be empty string");

		std::vector<Win32::PCCERT_CHAIN_CONTEXT> returnValue;
		std::vector<std::byte> encodedIssuer = EncodeAsnString(issuer);
		Win32::CERT_NAME_BLOB nameBlob{
			.cbData = static_cast<Win32::DWORD>(encodedIssuer.size()),
			.pbData = reinterpret_cast<Win32::BYTE*>(&encodedIssuer[0])
		};
		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/ns-wincrypt-cert_chain_find_by_issuer_para
		Win32::CERT_CHAIN_FIND_BY_ISSUER_PARA findParams{
			.cbSize = sizeof(findParams),
			.pszUsageIdentifier = 0, //szOID_PKIX_KP_CLIENT_AUTH,
			.dwKeySpec = 0,
			.cIssuer = 1,
			.rgIssuer = &nameBlob
		};
		Win32::PCCERT_CHAIN_CONTEXT chain = nullptr;
		while (true)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindchaininstore
			chain = Win32::CertFindChainInStore(
				hCertStore,
				Win32::X509AsnEncoding,
				0, // dwFindFlags,
				Win32::CertChainFindByIssuer, // dwFindType,
				&findParams,
				chain
			);
			// We assume no further matches were found. CertFindChainInStore() 
			// does not give any specific indication if it failed or not.
			if (not chain)
				return returnValue;
			// CertFindChainInStore frees the chain in each call, so we need
			// to duplicate it to retain a valid handle.
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certduplicatecertificatechain
			Win32::PCCERT_CHAIN_CONTEXT duplicate = Win32::CertDuplicateCertificateChain(chain);
			if (not duplicate)
			{
				// For some reason, we've failed to duplicate the chain; delete 
				// the current chain and bail. Like CertFindChainInStore(),
				// CertDuplicateCertificateChain() does not appear to provide any 
				// mechanism to determine why it failed.
				Win32::CertFreeCertificateChain(chain);
				throw Error::Win32Error("CertDuplicateCertificateChain() failed");
			}
			returnValue.push_back(duplicate);
		}
	}

	Win32::PCCERT_CHAIN_CONTEXT GenerateChainFrom(
		Win32::PCCERT_CONTEXT contextToBuildFrom,
		Win32::HCERTSTORE store
	)
	{
		if (not contextToBuildFrom)
			throw Error::Boring32Error("contextToBuildFrom is null");

		Win32::PCCERT_CHAIN_CONTEXT chainContext = nullptr;
		Win32::CERT_ENHKEY_USAGE enhkeyUsage{
			.cUsageIdentifier = 0,
			.rgpszUsageIdentifier = nullptr
		};
		Win32::CERT_USAGE_MATCH certUsage{
			.dwType = Win32::UsageMatchTypeAnd,
			.Usage = enhkeyUsage
		};
		Win32::CERT_CHAIN_PARA certChainParams{
			.cbSize = sizeof(certChainParams),
			.RequestedUsage = certUsage
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetcertificatechain
		// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-creating-a-certificate-chain
		bool succeeded = Win32::CertGetCertificateChain(
			nullptr,
			contextToBuildFrom,
			nullptr,
			store,
			&certChainParams,
			Win32::CertChainRevocationCheckChainExcludeRoot,
			nullptr,
			&chainContext
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CertGetCertificateChain() failed");
		return chainContext;
	}

	void ImportCertToStore(const Win32::HCERTSTORE store, const Win32::CRYPTUI_WIZ_IMPORT_SRC_INFO& info)
	{
		if (not store)
			throw Error::Boring32Error("store is nullptr");

		constexpr Win32::DWORD flags =
			Win32::CryptUiWizNoUi | Win32::CryptUiWizIgnoreNoUiFlagForCsps | Win32::CryptUiWizImportAllowCert;

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizimport
		bool succeeded = Win32::CryptUIWizImport(
			flags,
			nullptr,
			nullptr,
			&info,
			store
		);
		if (not succeeded)
			throw Error::Win32Error(Win32::GetLastError(), "CryptUIWizImport() failed");
	}

	using StoreFindType = Win32::StoreFindType;

	Win32::PCCERT_CONTEXT GetCertByArg(Win32::HCERTSTORE certStore, StoreFindType searchFlag, const void* arg)
	{
		if (not certStore)
			throw Error::Boring32Error("CertStore cannot be null");

		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindcertificateinstore
		Win32::PCCERT_CONTEXT certContext = static_cast<Win32::PCCERT_CONTEXT>(
			Win32::CertFindCertificateInStore(
				certStore,
				Win32::X509AsnEncoding | Win32::Pkcs7AsnEncoding | Win32::CertFindHasPrivateKey,
				0,
				static_cast<Win32::DWORD>(searchFlag),
				arg,
				nullptr
			)
		);
		if (not certContext)
			if (auto lastError = Win32::GetLastError(); lastError != Win32::CryptoErrorCodes::NotFound)
				throw Error::Win32Error(lastError, "CertFindCertificateInStore() failed");

		return certContext;
	}
}
export module boring32.crypto:functions;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.error;
import :cryptokey;

export namespace Boring32::Crypto
{
	// See also a complete example on MSDN at:
	// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-using-cryptprotectdata

	enum class EncryptOptions
	{
		LocalMachine = CRYPTPROTECT_LOCAL_MACHINE,
		UiForbidden = CRYPTPROTECT_UI_FORBIDDEN,
		Audit = CRYPTPROTECT_AUDIT
	};

	enum class DecryptOptions
	{
		UiForbidden = CRYPTPROTECT_UI_FORBIDDEN,
		VerifyProtection = CRYPTPROTECT_VERIFY_PROTECTION
	};

	std::vector<std::byte> Encrypt(
		const std::vector<std::byte>& data,
		const std::wstring& password,
		const std::wstring& description
	)
	{
		DATA_BLOB dataIn{
			.cbData = static_cast<DWORD>(data.size()),
			.pbData = reinterpret_cast<BYTE*>(const_cast<std::byte*>(&data[0]))
		};

		DATA_BLOB additionalEntropy{ 0 };
		if (!password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<DWORD>(password.size() * sizeof(wchar_t));
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
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CryptProtectData() failed", lastError);
		}

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
		if (!password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<DWORD>(password.size() * sizeof(wchar_t));
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
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CryptUnprotectData() failed", lastError);
		}

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

	std::vector<std::byte> Decrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText,
		const DWORD flags
	)
	{
		if (!key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (!iv.empty())
		{
			// Do all cipher algs require this?
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block length");
			pIV = reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<ULONG>(iv.size());
		}

		// Determine the byte size of the decrypted data
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		NTSTATUS status = BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NTStatusError("BCryptDecrypt() failed to count bytes", status);

		// Actually do the decryption
		std::vector<std::byte> plainText(cbData, std::byte{ 0 });
		status = BCryptDecrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&cypherText[0])),
			static_cast<ULONG>(cypherText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<PUCHAR>(&plainText[0]),
			static_cast<ULONG>(plainText.size()),
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NTStatusError("BCryptDecrypt() failed to decrypt", status);

		plainText.resize(cbData);
		return plainText;
	}

	std::vector<std::byte> Encrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText,
		const DWORD flags
	)
	{
		if (!key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		PUCHAR pIV = nullptr;
		ULONG ivSize = 0;
		if (!iv.empty())
		{
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block lenth");
			pIV = reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<ULONG>(iv.size());
		}

		// Determine the byte size of the encrypted data
		DWORD cbData = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		NTSTATUS status = BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			nullptr,
			0,
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NTStatusError("BCryptEncrypt() failed to count bytes", status);

		// Actually do the encryption
		std::vector<std::byte> cypherText(cbData, std::byte{ 0 });
		status = BCryptEncrypt(
			key.GetHandle(),
			reinterpret_cast<PUCHAR>(const_cast<std::byte*>(&plainText[0])),
			static_cast<ULONG>(plainText.size()),
			nullptr,
			pIV,
			ivSize,
			reinterpret_cast<PUCHAR>(&cypherText[0]),
			static_cast<ULONG>(cypherText.size()),
			&cbData,
			flags
		);
		if (!BCRYPT_SUCCESS(status))
			throw Error::NTStatusError("BCryptEncrypt() failed to encrypt", status);

		return cypherText;
	}

	std::string ToBase64String(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		DWORD size = 0;
		bool succeeded = CryptBinaryToStringA(
			reinterpret_cast<BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<DWORD>(bytes.size()),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			nullptr,
			&size
		);
		if (!succeeded)
			throw Error::Win32Error("CryptBinaryToStringA() failed when calculating size");
		if (size == 0)
			return "";

		std::string returnVal(size, L'\0');
		succeeded = CryptBinaryToStringA(
			reinterpret_cast<BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<DWORD>(bytes.size()),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			static_cast<LPSTR>(&returnVal[0]),
			&size
		);
		if (!succeeded)
			throw Error::Win32Error("CryptBinaryToStringA() failed when encoding");
		// Remove terminating null character
		if (!returnVal.empty())
			returnVal.pop_back();

		return returnVal;
	}

	std::wstring ToBase64WString(const std::vector<std::byte>& bytes)
	{
		// Determine the required size -- this includes the null terminator
		DWORD size = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-cryptbinarytostringw
		bool succeeded = CryptBinaryToStringW(
			reinterpret_cast<BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<DWORD>(bytes.size()),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			nullptr,
			&size
		);
		if (!succeeded)
			throw Error::Win32Error("CryptBinaryToStringW() failed when calculating size");
		if (size == 0)
			return L"";

		std::wstring returnVal(size, L'\0');
		succeeded = CryptBinaryToStringW(
			reinterpret_cast<BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<DWORD>(bytes.size()),
			CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
			static_cast<LPWSTR>(&returnVal[0]),
			&size
		);
		if (!succeeded)
			throw Error::Win32Error("CryptBinaryToStringW() failed when encoding");
		// Remove terminating null character
		if (!returnVal.empty())
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
		if (!succeeded)
			throw Error::Win32Error("CryptStringToBinaryW() failed when calculating size");

		std::vector<std::byte> returnVal(byteSize);
		succeeded = CryptStringToBinaryW(
			&base64[0],
			0,
			CRYPT_STRING_BASE64,
			reinterpret_cast<BYTE*>(&returnVal[0]),
			&byteSize,
			nullptr,
			nullptr
		);
		if (!succeeded)
			throw Error::Win32Error("CryptStringToBinaryW() failed when decoding");

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
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CertStrToNameW() failed", lastError);
		}

		std::vector<std::byte> bytes(encoded);
		succeeded = CertStrToNameW(
			X509_ASN_ENCODING,
			name.c_str(),
			flags,
			nullptr,
			reinterpret_cast<BYTE*>(&bytes[0]),
			&encoded,
			nullptr
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CertStrToNameW() failed", lastError);
		}

		bytes.resize(encoded);
		return bytes;
	}

	std::wstring FormatAsnNameBlob(
		const CERT_NAME_BLOB& certName,
		const DWORD format
	)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certnametostrw
		DWORD characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB*>(&certName),
			format,
			nullptr,
			0
		);
		if (characterSize == 0)
			return L"";

		std::wstring name(characterSize, '\0');
		characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			const_cast<CERT_NAME_BLOB*>(&certName),
			format,
			&name[0],
			static_cast<DWORD>(name.size())
		);
		name.pop_back(); // remove excess null character

		return name;
	}

	std::vector<PCCERT_CHAIN_CONTEXT> FindChainInStore(
		HCERTSTORE hCertStore,
		const std::wstring& issuer
	)
	{
		if (!hCertStore)
			throw Error::Boring32Error("hCertStore cannot be null");
		if (issuer.empty())
			throw Error::Boring32Error("issuer cannot be empty string");

		std::vector<PCCERT_CHAIN_CONTEXT> returnValue;
		std::vector<std::byte> encodedIssuer = EncodeAsnString(issuer);
		CERT_NAME_BLOB nameBlob{
			.cbData = static_cast<DWORD>(encodedIssuer.size()),
			.pbData = reinterpret_cast<BYTE*>(&encodedIssuer[0])
		};
		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/ns-wincrypt-cert_chain_find_by_issuer_para
		CERT_CHAIN_FIND_BY_ISSUER_PARA findParams{
			.cbSize = sizeof(findParams),
			.pszUsageIdentifier = 0, //szOID_PKIX_KP_CLIENT_AUTH,
			.dwKeySpec = 0,
			.cIssuer = 1,
			.rgIssuer = &nameBlob
		};
		PCCERT_CHAIN_CONTEXT chain = nullptr;
		while (true)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindchaininstore
			chain = CertFindChainInStore(
				hCertStore,
				X509_ASN_ENCODING,
				0, // dwFindFlags,
				CERT_CHAIN_FIND_BY_ISSUER, // dwFindType,
				&findParams,
				chain
			);
			// We assume no further matches were found. CertFindChainInStore() 
			// does not give any specific indication if it failed or not.
			if (!chain)
				return returnValue;
			// CertFindChainInStore frees the chain in each call, so we need
			// to duplicate it to retain a valid handle.
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certduplicatecertificatechain
			PCCERT_CHAIN_CONTEXT duplicate = CertDuplicateCertificateChain(chain);
			if (!duplicate)
			{
				// For some reason, we've failed to duplicate the chain; delete 
				// the current chain and bail. Like CertFindChainInStore(),
				// CertDuplicateCertificateChain() does not appear to provide any 
				// mechanism to determine why it failed.
				CertFreeCertificateChain(chain);
				throw Error::Win32Error("CertDuplicateCertificateChain() failed");
			}
			returnValue.push_back(duplicate);
		}
	}

	PCCERT_CHAIN_CONTEXT GenerateChainFrom(
		PCCERT_CONTEXT contextToBuildFrom,
		HCERTSTORE store
	)
	{
		if (!contextToBuildFrom)
			throw Error::Boring32Error("contextToBuildFrom is null");

		PCCERT_CHAIN_CONTEXT chainContext = nullptr;
		CERT_ENHKEY_USAGE enhkeyUsage{
			.cUsageIdentifier = 0,
			.rgpszUsageIdentifier = nullptr
		};
		CERT_USAGE_MATCH certUsage{
			.dwType = USAGE_MATCH_TYPE_AND,
			.Usage = enhkeyUsage
		};
		CERT_CHAIN_PARA certChainParams{
			.cbSize = sizeof(certChainParams),
			.RequestedUsage = certUsage
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetcertificatechain
		// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-creating-a-certificate-chain
		const bool succeeded = CertGetCertificateChain(
			nullptr,
			contextToBuildFrom,
			nullptr,
			store,
			&certChainParams,
			CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
			nullptr,
			&chainContext
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CertGetCertificateChain() failed", lastError);
		}
		return chainContext;
	}


	void ImportCertToStore(
		const HCERTSTORE store, 
		const CRYPTUI_WIZ_IMPORT_SRC_INFO& info
	)
	{
		if (!store)
			throw Error::Boring32Error("store is nullptr");

		constexpr DWORD CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS = 0x0002;
		constexpr DWORD flags = CRYPTUI_WIZ_NO_UI
			| CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS
			| CRYPTUI_WIZ_IMPORT_ALLOW_CERT;

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizimport
		const bool succeeded = CryptUIWizImport(
			flags,
			nullptr,
			nullptr,
			&info,
			store
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"CryptUIWizImport() failed",
				lastError
			);
		}
	}

	// See https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindcertificateinstore
	enum class StoreFindType : DWORD
	{
		Any = CERT_FIND_ANY,
		CertId = CERT_FIND_CERT_ID,
		CtlUsage = CERT_FIND_CTL_USAGE,
		EnhKeyUsage = CERT_FIND_ENHKEY_USAGE,
		Existing = CERT_FIND_EXISTING,
		Hash = CERT_FIND_HASH,
		HasPrivateKey = CERT_FIND_HAS_PRIVATE_KEY,
		IssuerAttr = CERT_FIND_ISSUER_ATTR,
		IssuerName = CERT_FIND_ISSUER_NAME,
		IssuerOf = CERT_FIND_ISSUER_OF,
		IssuerStr = CERT_FIND_ISSUER_STR,
		KeyIdentifier = CERT_FIND_KEY_IDENTIFIER,
		KeySpec = CERT_FIND_KEY_SPEC,
		Md5Hash = CERT_FIND_MD5_HASH,
		FindProperty = CERT_FIND_PROPERTY,
		PublicKey = CERT_FIND_PUBLIC_KEY,
		Sha1Hash = CERT_FIND_SHA1_HASH,
		SignatureHash = CERT_FIND_SIGNATURE_HASH,
		SubjectAttr = CERT_FIND_SUBJECT_ATTR,
		SubjectCert = CERT_FIND_SUBJECT_CERT,
		SubjectName = CERT_FIND_SUBJECT_NAME,
		SubjectStr = CERT_FIND_SUBJECT_STR,
		CrossCertDistPoints = CERT_FIND_CROSS_CERT_DIST_POINTS,
		PubKeyMd5Hash = CERT_FIND_PUBKEY_MD5_HASH
	};

	PCCERT_CONTEXT GetCertByArg(
		HCERTSTORE certStore,
		const StoreFindType searchFlag,
		const void* arg
	)
	{
		if (!certStore)
			throw Error::Boring32Error("CertStore cannot be null");

		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindcertificateinstore
		PCCERT_CONTEXT certContext = static_cast<PCCERT_CONTEXT>(
			CertFindCertificateInStore(
				certStore,
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING | CERT_FIND_HAS_PRIVATE_KEY,
				0,
				static_cast<DWORD>(searchFlag),
				arg,
				nullptr
			)
		);
		if (!certContext)
		{
			const DWORD lastError = GetLastError();
			if (lastError != CRYPT_E_NOT_FOUND)
				throw Error::Win32Error("CertFindCertificateInStore() failed", lastError);
		}

		return certContext;
	}
}
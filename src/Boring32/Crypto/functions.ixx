export module boring32:crypto.functions;
import std;
import :win32;
import :error;
import :crypto.cryptokey;

export namespace Boring32::Crypto
{
	// See also a complete example on MSDN at:
	// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-using-cryptprotectdata

	auto Encrypt(
		const std::vector<std::byte>& data,
		const std::wstring& password,
		const std::wstring& description
	) -> std::vector<std::byte>
	{
		auto dataIn = Win32::DATA_BLOB{
			.cbData = static_cast<Win32::DWORD>(data.size()),
			.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&data[0]))
		};

		auto additionalEntropy = Win32::DATA_BLOB{ 0 };
		if (not password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<Win32::DWORD>(password.size() * sizeof(wchar_t));
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
		auto encryptedBlob = Win32::DATA_BLOB{ 0 };
		auto descriptionCStr = description.empty()
			? nullptr
			: description.c_str();
		auto entropy = password.empty()
			? static_cast<Win32::DATA_BLOB*>(nullptr)
			: &additionalEntropy;
		auto succeeded = 
			Win32::CryptProtectData(
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
			throw Error::Win32Error{Win32::GetLastError(), "CryptProtectData() failed"};

		auto returnValue = std::vector<std::byte>{
			reinterpret_cast<std::byte*>(encryptedBlob.pbData),
			reinterpret_cast<std::byte*>(encryptedBlob.pbData) + encryptedBlob.cbData
		};
		if (encryptedBlob.pbData)
			Win32::LocalFree(encryptedBlob.pbData);

		return returnValue;
	}

	auto Encrypt(
		const std::wstring& str,
		const std::wstring& password,
		const std::wstring& description
	) -> std::vector<std::byte>
	{
		auto buffer = reinterpret_cast<const std::byte*>(str.data());
		return Encrypt(
			std::vector<std::byte>(buffer, buffer + str.size() * sizeof(wchar_t)),
			password,
			description
		);
	}

	auto DecryptString(
		const std::vector<std::byte>& encryptedData,
		const std::wstring& password,
		std::wstring& outDescription
	) -> std::wstring
	{
		auto encryptedBlob = Win32::DATA_BLOB{
			.cbData = static_cast<Win32::DWORD>(encryptedData.size()),
			.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>((encryptedData.data())))
		};

		auto additionalEntropy = Win32::DATA_BLOB{ 0 };
		if (not password.empty())
		{
			additionalEntropy.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(&password[0]));
			additionalEntropy.cbData = static_cast<Win32::DWORD>(password.size() * sizeof(wchar_t));
		}

		auto decryptedBlob = Win32::DATA_BLOB{};
		auto descrOut = (Win32::LPWSTR)nullptr;
		auto entropy = password.empty()
			? (Win32::DATA_BLOB*)nullptr
			: &additionalEntropy;
		// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
		auto succeeded = 
			Win32::CryptUnprotectData(
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
			throw Error::Win32Error{Win32::GetLastError(), "CryptUnprotectData() failed"};

		if (descrOut)
		{
			outDescription = descrOut;
			Win32::LocalFree(descrOut);
		}

		auto returnValue = std::wstring(
			reinterpret_cast<wchar_t*>(decryptedBlob.pbData),
			decryptedBlob.cbData / sizeof(wchar_t)
		);
		if (decryptedBlob.pbData)
			Win32::LocalFree(decryptedBlob.pbData);

		return returnValue;
	}

	auto Decrypt(
		const Win32::DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText,
		const Win32::DWORD flags
	) -> std::vector<std::byte>
	{
		if (not key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		auto pIV = (Win32::PUCHAR)nullptr;
		auto ivSize = Win32::ULONG{};
		if (not iv.empty())
		{
			// Do all cipher algs require this?
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block length");
			pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<Win32::ULONG>(iv.size());
		}

		// Determine the byte size of the decrypted data
		auto cbData = Win32::DWORD{};
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdecrypt
		auto status = 
			Win32::BCryptDecrypt(
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
		auto plainText = std::vector<std::byte>(cbData, std::byte{ 0 });
		status = 
			Win32::BCryptDecrypt(
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

	auto Encrypt(
		Win32::DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText,
		Win32::DWORD flags
	) -> std::vector<std::byte>
	{
		if (not key.GetHandle())
			throw Error::Boring32Error("key is null");

		// IV is optional
		auto pIV = (Win32::PUCHAR)nullptr;
		auto ivSize = Win32::ULONG{};
		if (not iv.empty())
		{
			if (iv.size() != blockByteLength)
				throw Error::Boring32Error("IV must be the same size as the AES block lenth");
			pIV = reinterpret_cast<Win32::PUCHAR>(const_cast<std::byte*>(&iv[0]));
			ivSize = static_cast<Win32::ULONG>(iv.size());
		}

		// Determine the byte size of the encrypted data
		auto cbData = Win32::DWORD{};
		// https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptencrypt
		auto status = 
			Win32::BCryptEncrypt(
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
		auto cypherText = std::vector<std::byte>(cbData, std::byte{ 0 });
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

	auto ToBase64String(const std::vector<std::byte>& bytes) -> std::string
	{
		// Determine the required size -- this includes the null terminator
		auto size = Win32::DWORD{};
		constexpr auto flags = Win32::DWORD{Win32::CryptStringBase64 | Win32::CryptStringNoCrLf};
		auto succeeded = 
			Win32::CryptBinaryToStringA(
				reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
				static_cast<Win32::DWORD>(bytes.size()),
				flags,
				nullptr,
				&size
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptBinaryToStringA() failed when calculating size"};
		if (size == 0)
			return {};

		auto returnVal = std::string(size, L'\0');
		succeeded = 
			Win32::CryptBinaryToStringA(
				reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
				static_cast<Win32::DWORD>(bytes.size()),
				flags,
				static_cast<Win32::LPSTR>(&returnVal[0]),
				&size
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptBinaryToStringA() failed when encoding"};
		// Remove terminating null character
		if (not returnVal.empty())
			returnVal.pop_back();

		return returnVal;
	}

	auto ToBase64WString(const std::vector<std::byte>& bytes) -> std::wstring
	{
		// Determine the required size -- this includes the null terminator
		auto size = Win32::DWORD{};
		constexpr auto flags = Win32::DWORD{ Win32::CryptStringBase64 | Win32::CryptStringNoCrLf };
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-cryptbinarytostringw
		auto succeeded = 
			Win32::CryptBinaryToStringW(
				reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
				static_cast<Win32::DWORD>(bytes.size()),
				flags,
				nullptr,
				&size
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptBinaryToStringW() failed when calculating size"};
		if (size == 0)
			return {};

		auto returnVal = std::wstring(size, L'\0');
		succeeded = Win32::CryptBinaryToStringW(
			reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&bytes[0])),
			static_cast<Win32::DWORD>(bytes.size()),
			flags,
			static_cast<Win32::LPWSTR>(&returnVal[0]),
			&size
		);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptBinaryToStringW() failed when encoding"};
		// Remove terminating null character
		if (not returnVal.empty())
			returnVal.pop_back();

		return returnVal;
	}

	auto ToBinary(const std::wstring& base64) -> std::vector<std::byte>
	{
		auto byteSize = Win32::DWORD{};
		auto succeeded = 
			Win32::CryptStringToBinaryW(
				&base64[0],
				0,
				Win32::CryptStringBase64,
				nullptr,
				&byteSize,
				nullptr,
				nullptr
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptStringToBinaryW() failed when calculating size"};

		auto returnVal = std::vector<std::byte>(byteSize);
		succeeded = 
			Win32::CryptStringToBinaryW(
				&base64[0],
				0,
				Win32::CryptStringBase64,
				reinterpret_cast<Win32::BYTE*>(&returnVal[0]),
				&byteSize,
				nullptr,
				nullptr
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptStringToBinaryW() failed when decoding"};

		returnVal.resize(byteSize);
		return returnVal;
	}

	auto EncodeAsnString(const std::wstring& name) -> std::vector<std::byte>
	{
		auto encoded = Win32::DWORD{};
		// CERT_NAME_STR_FORCE_UTF8_DIR_STR_FLAG is required or the encoding
		// produces subtle differences in the encoded bytes (DC3 vs FF in 
		// original buffer), which causes the match to fail
		// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certstrtonamew
		constexpr auto flags = Win32::DWORD{ Win32::CertX500NameStr | Win32::CertNameStrForceUtf8DirStrFlag };
		auto succeeded = 
			Win32::CertStrToNameW(
				Win32::X509AsnEncoding,
				name.c_str(),
				flags,
				nullptr,
				nullptr,
				&encoded,
				nullptr
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CertStrToNameW() failed"};

		 auto bytes = std::vector<std::byte>(encoded);
		succeeded = 
			Win32::CertStrToNameW(
				Win32::X509AsnEncoding,
				name.c_str(),
				flags,
				nullptr,
				reinterpret_cast<Win32::BYTE*>(&bytes[0]),
				&encoded,
				nullptr
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CertStrToNameW() failed"};

		bytes.resize(encoded);
		return bytes;
	}

	auto FormatAsnNameBlob(const Win32::CERT_NAME_BLOB& certName, Win32::DWORD format) -> std::wstring
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certnametostrw
		auto characterSize = Win32::DWORD{
			Win32::CertNameToStrW(
				Win32::X509AsnEncoding,
				const_cast<Win32::CERT_NAME_BLOB*>(&certName),
				format,
				nullptr,
				0
			) };
		if (characterSize == 0)
			return L"";

		auto name = std::wstring(characterSize, L'\0');
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

	auto FindChainInStore(Win32::HCERTSTORE hCertStore, const std::wstring& issuer) -> std::vector<Win32::PCCERT_CHAIN_CONTEXT>
	{
		if (not hCertStore)
			throw Error::Boring32Error("hCertStore cannot be null");
		if (issuer.empty())
			throw Error::Boring32Error("issuer cannot be empty string");

		auto returnValue = std::vector<Win32::PCCERT_CHAIN_CONTEXT>{};
		auto encodedIssuer = std::vector<std::byte>{ EncodeAsnString(issuer) };
		auto nameBlob = Win32::CERT_NAME_BLOB{
			.cbData = static_cast<Win32::DWORD>(encodedIssuer.size()),
			.pbData = reinterpret_cast<Win32::BYTE*>(&encodedIssuer[0])
		};
		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/ns-wincrypt-cert_chain_find_by_issuer_para
		auto findParams = Win32::CERT_CHAIN_FIND_BY_ISSUER_PARA{
			.cbSize = sizeof(Win32::CERT_CHAIN_FIND_BY_ISSUER_PARA),
			.pszUsageIdentifier = 0, //szOID_PKIX_KP_CLIENT_AUTH,
			.dwKeySpec = 0,
			.cIssuer = 1,
			.rgIssuer = &nameBlob
		};
		auto chain = Win32::PCCERT_CHAIN_CONTEXT{ nullptr };
		while (true)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindchaininstore
			chain = 
				Win32::CertFindChainInStore(
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
			auto duplicate = Win32::PCCERT_CHAIN_CONTEXT{Win32::CertDuplicateCertificateChain(chain)};
			if (not duplicate)
			{
				// For some reason, we've failed to duplicate the chain; delete 
				// the current chain and bail. Like CertFindChainInStore(),
				// CertDuplicateCertificateChain() does not appear to provide any 
				// mechanism to determine why it failed.
				Win32::CertFreeCertificateChain(chain);
				throw Error::Win32Error{Win32::GetLastError(), "CertDuplicateCertificateChain() failed"};
			}
			returnValue.push_back(duplicate);
		}
	}

	auto GenerateChainFrom(
		Win32::PCCERT_CONTEXT contextToBuildFrom,
		Win32::HCERTSTORE store
	) -> Win32::PCCERT_CHAIN_CONTEXT
	{
		if (not contextToBuildFrom)
			throw Error::Boring32Error("contextToBuildFrom is null");

		auto chainContext = Win32::PCCERT_CHAIN_CONTEXT{ nullptr };
		auto enhkeyUsage = Win32::CERT_ENHKEY_USAGE{
			.cUsageIdentifier = 0,
			.rgpszUsageIdentifier = nullptr
		};
		auto certUsage = Win32::CERT_USAGE_MATCH{
			.dwType = Win32::UsageMatchTypeAnd,
			.Usage = enhkeyUsage
		};
		auto certChainParams = Win32::CERT_CHAIN_PARA{
			.cbSize = sizeof(Win32::CERT_CHAIN_PARA),
			.RequestedUsage = certUsage
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetcertificatechain
		// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-creating-a-certificate-chain
		auto succeeded = 
			Win32::CertGetCertificateChain(
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
			throw Error::Win32Error{Win32::GetLastError(), "CertGetCertificateChain() failed"};
		return chainContext;
	}

	void ImportCertToStore(const Win32::HCERTSTORE store, const Win32::CRYPTUI_WIZ_IMPORT_SRC_INFO& info)
	{
		if (not store)
			throw Error::Boring32Error{"store is nullptr"};

		constexpr auto flags = Win32::DWORD{
			Win32::CryptUiWizNoUi | Win32::CryptUiWizIgnoreNoUiFlagForCsps | Win32::CryptUiWizImportAllowCert
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizimport
		auto succeeded = 
			Win32::CryptUIWizImport(
				flags,
				nullptr,
				nullptr,
				&info,
				store
			);
		if (not succeeded)
			throw Error::Win32Error{Win32::GetLastError(), "CryptUIWizImport() failed"};
	}

	using StoreFindType = Win32::StoreFindType;

	auto GetCertByArg(Win32::HCERTSTORE certStore, StoreFindType searchFlag, const void* arg) -> Win32::PCCERT_CONTEXT
	{
		if (not certStore)
			throw Error::Boring32Error("CertStore cannot be null");

		// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certfindcertificateinstore
		auto certContext = Win32::PCCERT_CONTEXT{
			Win32::CertFindCertificateInStore(
				certStore,
				Win32::X509AsnEncoding | Win32::Pkcs7AsnEncoding | Win32::CertFindHasPrivateKey,
				0,
				static_cast<Win32::DWORD>(searchFlag),
				arg,
				nullptr
			) };
		if (not certContext)
			if (auto lastError = Win32::GetLastError(); lastError != Win32::CryptoErrorCodes::NotFound)
				throw Error::Win32Error{lastError, "CertFindCertificateInStore() failed"};

		return certContext;
	}
}
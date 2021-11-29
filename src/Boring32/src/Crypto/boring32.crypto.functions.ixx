module;

#include <string>
#include <vector>
#include <Windows.h>
//#include <bcrypt.h>
#include <dpapi.h>

export module boring32.crypto.functions;
import boring32.crypto.cryptokey;

export namespace Boring32::Crypto
{
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
	);

	std::vector<std::byte> Encrypt(
		const std::wstring& str,
		const std::wstring& password,
		const std::wstring& description
	);

	std::wstring DecryptString(
		const std::vector<std::byte>& encryptedData,
		const std::wstring& password,
		std::wstring& outDescription
	);

	std::vector<std::byte> Decrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& cypherText,
		const DWORD flags
	);

	std::vector<std::byte> Encrypt(
		const DWORD blockByteLength,
		const CryptoKey& key,
		const std::vector<std::byte>& iv,
		const std::vector<std::byte>& plainText,
		const DWORD flags
	);

	std::string ToBase64String(const std::vector<std::byte>& bytes);
	std::wstring ToBase64WString(const std::vector<std::byte>& bytes);
	std::vector<std::byte> ToBinary(const std::wstring& base64);
	std::vector<std::byte> EncodeAsnString(const std::wstring& name);
	std::wstring FormatAsnNameBlob(
		const CERT_NAME_BLOB& certName,
		const DWORD format
	);
}
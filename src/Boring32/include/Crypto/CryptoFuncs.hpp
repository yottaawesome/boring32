#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace Boring32::Crypto
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
}
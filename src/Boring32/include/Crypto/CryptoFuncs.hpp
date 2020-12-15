#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace Boring32::Crypto
{
	std::vector<std::byte> EncryptString(
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
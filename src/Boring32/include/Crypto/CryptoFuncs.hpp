#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace Boring32::Crypto
{
	std::vector<std::byte> EncryptString(
		const std::wstring& description,
		const std::wstring& str
	);

	std::wstring DecryptString(
		const std::vector<std::byte>& encryptedData,
		std::wstring& outDescription
	);
}
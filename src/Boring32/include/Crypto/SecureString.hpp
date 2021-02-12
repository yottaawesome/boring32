#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace Boring32::Crypto
{
	enum class EncryptionType : DWORD
	{
		SameProcess = CRYPTPROTECTMEMORY_SAME_PROCESS,
		CrossProcess = CRYPTPROTECTMEMORY_CROSS_PROCESS,
		SameLogon = CRYPTPROTECTMEMORY_SAME_LOGON
	};

	class SecureString
	{
		public:
			virtual ~SecureString();
			SecureString();
			SecureString(const std::wstring& value);
			SecureString(const std::wstring& value, const EncryptionType encryptionType);

		public:
			virtual void SetValue(const std::wstring& value);
			virtual void GetValue(std::wstring& value);
			virtual void Clear();
			virtual bool HasData() const noexcept;

		protected:
			EncryptionType m_encryptionType;
			DWORD m_characters;
			std::vector<std::byte> m_encryptedData;
	};
}
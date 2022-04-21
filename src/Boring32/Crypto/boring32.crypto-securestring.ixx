module;

#include <string>
#include <vector>
#include <Windows.h>
#include <dpapi.h>

export module boring32.crypto:securestring;

export namespace Boring32::Crypto
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
			virtual operator bool() const noexcept;
			virtual explicit operator std::wstring();
			virtual SecureString& operator=(const std::wstring& newValue);

		public:
			virtual void SetValueAndEncrypt(const std::wstring& value);
			virtual void DecryptAndCopy(std::wstring& value);
			virtual const std::wstring& GetValue() const;
			virtual void Clear();
			virtual bool HasData() const noexcept;
			virtual void Encrypt();
			virtual void Decrypt();
			virtual bool IsCurrentlyEncrypted() const noexcept;

		protected:
			EncryptionType m_encryptionType;
			DWORD m_characters;
			std::wstring m_protectedString;
			bool m_isEncrypted;
	};
}
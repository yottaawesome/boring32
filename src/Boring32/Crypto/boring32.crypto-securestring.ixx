export module boring32.crypto:securestring;
import <string>;
import <vector>;
import <win32.hpp>;
import boring32.error;
import <algorithm>;

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
			virtual ~SecureString()
			{
				Clear();
			}

			SecureString() = default;

			SecureString(const std::wstring& value)
			{
				SetValueAndEncrypt(value);
			}

			SecureString(const std::wstring& value, const EncryptionType encryptionType)
			{
				SetValueAndEncrypt(value);
			}

		public:
			virtual operator bool() const noexcept
			{
				return m_isEncrypted;
			}

			virtual explicit operator std::wstring()
			{
				std::wstring out;
				DecryptAndCopy(out);
				return out;
			}

			virtual SecureString& operator=(const std::wstring& newValue)
			{
				SetValueAndEncrypt(newValue);
				return *this;
			}

		public:
			virtual void SetValueAndEncrypt(const std::wstring& value)
			{
				Clear();

				// We need the character count when the client asks to
				// decrypt. Strictly speaking, keeping this data
				// violates Shannon's perfect secrecy rule on encrypted 
				// data, but we can live with it
				m_characters = (DWORD)value.size();

				// Figure out if we need to align to the encryption block size
				const DWORD bytesOfPlainData = (DWORD)value.size() * sizeof(wchar_t);
				const DWORD bytesModBlock = bytesOfPlainData % CRYPTPROTECTMEMORY_BLOCK_SIZE;
				const DWORD bytesOfEncryptedData = bytesModBlock
					? bytesOfPlainData + CRYPTPROTECTMEMORY_BLOCK_SIZE - bytesModBlock
					: bytesOfPlainData;

				// Copy our plain data across to the buffer that will be
				// encrypted in-place
				m_protectedString = value;

				// Resize to a multiple of the encryption block size if
				// we need to
				if (bytesOfEncryptedData > bytesOfPlainData)
					m_protectedString.resize(bytesOfEncryptedData / sizeof(wchar_t));

				// Actually perform the encryption now
				// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectmemory
				Encrypt();
			}

			virtual void DecryptAndCopy(std::wstring& value)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectmemory
				// De-encrypt
				Decrypt();

				// Set the out parameter
				value = m_protectedString;
				value.resize(m_characters);

				// Re-encrypt
				Encrypt();
			}

			virtual const std::wstring& GetValue() const
			{
				return m_protectedString;
			}

			virtual void Clear()
			{
				std::fill(m_protectedString.begin(), m_protectedString.end(), '\0');
				m_protectedString.clear();
				m_characters = 0;
				m_isEncrypted = false;
			}

			virtual bool HasData() const noexcept
			{
				return m_protectedString.empty() == false;
			}

			virtual void Encrypt()
			{
				if (m_isEncrypted)
					return;
				if (m_protectedString.empty())
					throw Error::Boring32Error("Nothing to decrypt");

				const bool succeeded = CryptProtectMemory(
					(void*)&m_protectedString[0],
					(DWORD)m_protectedString.size() * sizeof(wchar_t),
					(DWORD)m_encryptionType
				);
				if (succeeded == false)
					throw Error::Win32Error("CryptProtectMemory() failed", GetLastError());
				m_isEncrypted = true;
			}

			virtual void Decrypt()
			{
				if (m_isEncrypted == false)
					return;
				if (m_protectedString.empty())
					throw Error::Boring32Error("Nothing to decrypt");

				const bool succeeded = CryptUnprotectMemory(
					(void*)&m_protectedString[0],
					(DWORD)m_protectedString.size() * sizeof(wchar_t),
					(DWORD)m_encryptionType
				);
				if (succeeded == false)
					throw Error::Win32Error("CryptUnprotectMemory() failed", GetLastError());
				m_isEncrypted = false;
			}

			virtual bool IsCurrentlyEncrypted() const noexcept
			{
				return m_isEncrypted;
			}

		protected:
			EncryptionType m_encryptionType = EncryptionType::SameProcess;
			DWORD m_characters = 0;
			std::wstring m_protectedString;
			bool m_isEncrypted = false;
	};
}
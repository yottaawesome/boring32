export module boring32.crypto:securestring;
import <string>;
import <vector>;
import <algorithm>; // for std::fill()
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Crypto
{
	enum class EncryptionType : DWORD
	{
		SameProcess = CRYPTPROTECTMEMORY_SAME_PROCESS,
		CrossProcess = CRYPTPROTECTMEMORY_CROSS_PROCESS,
		SameLogon = CRYPTPROTECTMEMORY_SAME_LOGON
	};

	class SecureString final
	{
		public:
			~SecureString()
			{
				Clear();
			}

			SecureString() = default;

			SecureString(const SecureString& other)
			{
				Copy(other);
			}
			SecureString& operator=(const SecureString& other)
			{
				return Copy(other);
			}

			SecureString(SecureString&& other) noexcept
			{
				Move(other);
			}
			SecureString& operator=(SecureString&& other) noexcept
			{
				return Move(other);
			}

		public:
			SecureString(const std::wstring& value)
			{
				SetValueAndEncrypt(value);
			}

			SecureString(const std::wstring& value, const EncryptionType encryptionType)
			{
				SetValueAndEncrypt(value);
			}

		public:
			operator bool() const noexcept
			{
				return m_isEncrypted;
			}

			explicit operator std::wstring()
			{
				std::wstring out;
				DecryptAndCopy(out);
				return out;
			}

			SecureString& operator=(const std::wstring& newValue)
			{
				SetValueAndEncrypt(newValue);
				return *this;
			}

		public:
			void SetValueAndEncrypt(const std::wstring& value)
			{
				Clear();

				// We need the character count when the client asks to
				// decrypt. Strictly speaking, keeping this data
				// violates Shannon's perfect secrecy rule on encrypted 
				// data, but we can live with it
				m_characters = static_cast<DWORD>(value.size());

				// Figure out if we need to align to the encryption block size
				const DWORD bytesOfPlainData = static_cast<DWORD>(value.size() * sizeof(wchar_t));
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

			void DecryptAndCopy(std::wstring& value)
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

			const std::wstring& GetValue() const
			{
				return m_protectedString;
			}

			void Clear()
			{
				std::fill(m_protectedString.begin(), m_protectedString.end(), '\0');
				m_protectedString.clear();
				m_characters = 0;
				m_isEncrypted = false;
			}

			bool HasData() const noexcept
			{
				return !m_protectedString.empty();
			}

			void Encrypt()
			{
				if (m_isEncrypted)
					return;
				if (m_protectedString.empty())
					throw Error::Boring32Error("Nothing to encrypt");

				const bool succeeded = CryptProtectMemory(
					reinterpret_cast<void*>(&m_protectedString[0]),
					static_cast<DWORD>(m_protectedString.size() * sizeof(wchar_t)),
					static_cast<DWORD>(m_encryptionType)
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CryptProtectMemory() failed", lastError);
				}
				m_isEncrypted = true;
			}

			void Decrypt()
			{
				if (!m_isEncrypted)
					return;
				if (m_protectedString.empty())
					throw Error::Boring32Error("Nothing to decrypt");

				const bool succeeded = CryptUnprotectMemory(
					reinterpret_cast<void*>(&m_protectedString[0]),
					static_cast<DWORD>(m_protectedString.size() * sizeof(wchar_t)),
					static_cast<DWORD>(m_encryptionType)
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CryptUnprotectMemory() failed", lastError);
				}
				m_isEncrypted = false;
			}

			bool IsCurrentlyEncrypted() const noexcept
			{
				return m_isEncrypted;
			}

		private:
			SecureString& Copy(const SecureString& other) noexcept
			{
				Clear();
				m_encryptionType = other.m_encryptionType;
				m_characters = other.m_characters;
				m_protectedString = other.m_protectedString;
				m_isEncrypted = other.m_isEncrypted;
				return *this;
			}

			SecureString& Move(SecureString& other) noexcept
			{
				Clear();
				m_encryptionType = other.m_encryptionType;
				m_characters = other.m_characters;
				m_protectedString = std::move(other.m_protectedString);
				m_isEncrypted = other.m_isEncrypted;

				other.m_characters = 0;
				other.m_isEncrypted = false;
				return *this;
			}

		private:
			EncryptionType m_encryptionType = EncryptionType::SameProcess;
			DWORD m_characters = 0;
			std::wstring m_protectedString;
			bool m_isEncrypted = false;
	};
}
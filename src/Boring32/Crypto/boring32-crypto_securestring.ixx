export module boring32:crypto_securestring;
import boring32.shared;
import boring32.error;

export namespace Boring32::Crypto
{
	class ScopedString final
	{
		public:
			~ScopedString()
			{
				Clear();
			}

			ScopedString() = default;

			ScopedString(const ScopedString& other) = default;
			ScopedString& operator=(const ScopedString& other)
			{
				Clear();
				Value = other.Value;
				return *this;
			}

			ScopedString(ScopedString&& other) noexcept = default;
			ScopedString& operator=(ScopedString&& other) noexcept
			{
				Clear();
				Value = std::move(other.Value);
				return *this;
			}

		public:
			ScopedString(const std::wstring& value)
				: Value(value)
			{ }

			ScopedString(std::wstring&& value)
				: Value(value)
			{ }

		public:
			void Clear()
			{
				std::fill(Value.begin(), Value.end(), '\0');
				Value.clear();
			}

		public:
			std::wstring Value;
	};

	class SecureString final
	{
		public:
			~SecureString()
			{
				Clear();
			}

			SecureString() = default;

			SecureString(const SecureString& other) = default;

			SecureString& operator=(const SecureString& other)
			{
				return Copy(other);
			}

			SecureString(SecureString&& other) noexcept = default;

			SecureString& operator=(SecureString&& other) noexcept
			{
				return Move(other);
			}

		public:
			SecureString(const std::wstring& value)
			{
				SetValueAndEncrypt(value);
			}

			SecureString(const std::wstring& value, const Win32::EncryptionType encryptionType)
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
				DecryptCopyAndReencrypt(out);
				return out;
			}

			SecureString& operator=(const std::wstring& newValue)
			{
				SetValueAndEncrypt(newValue);
				return *this;
			}

			bool operator==(const std::wstring& comparison)
			{
				ScopedString s;
				DecryptCopyAndReencrypt(s.Value);
				const bool comp = s.Value == comparison;
				return comp;
			}

		public:
			void SetValueAndEncrypt(const std::wstring& value)
			{
				Clear();

				// We need the character count when the client asks to
				// decrypt. Strictly speaking, keeping this data
				// violates Shannon's perfect secrecy rule on encrypted 
				// data, but we can live with it
				m_characters = static_cast<Win32::DWORD>(value.size());

				// Figure out if we need to align to the encryption block size
				const Win32::DWORD bytesOfPlainData = static_cast<Win32::DWORD>(value.size() * sizeof(wchar_t));
				const Win32::DWORD bytesModBlock = bytesOfPlainData % Win32::CryptProtectMemoryBlockSize;
				const Win32::DWORD bytesOfEncryptedData = bytesModBlock
					? bytesOfPlainData + Win32::CryptProtectMemoryBlockSize - bytesModBlock
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

			void DecryptCopyAndReencrypt(std::wstring& value)
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

			ScopedString ToScopedString()
			{
				ScopedString copy;
				DecryptCopyAndReencrypt(copy.Value);
				return copy;
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

				const bool succeeded = Win32::CryptProtectMemory(
					reinterpret_cast<void*>(&m_protectedString[0]),
					static_cast<Win32::DWORD>(m_protectedString.size() * sizeof(wchar_t)),
					static_cast<Win32::DWORD>(m_encryptionType)
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
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

				const bool succeeded = Win32::CryptUnprotectMemory(
					reinterpret_cast<void*>(&m_protectedString[0]),
					static_cast<Win32::DWORD>(m_protectedString.size() * sizeof(wchar_t)),
					static_cast<Win32::DWORD>(m_encryptionType)
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("CryptUnprotectMemory() failed", lastError);
				}
				m_isEncrypted = false;
			}

			bool IsCurrentlyEncrypted() const noexcept
			{
				return m_isEncrypted;
			}

		private:
			SecureString& Copy(const SecureString& other)
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
			Win32::EncryptionType m_encryptionType = Win32::EncryptionType::SameProcess;
			Win32::DWORD m_characters = 0;
			std::wstring m_protectedString;
			bool m_isEncrypted = false;
	};
}
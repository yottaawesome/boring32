export module boring32:security_token;
import std;
import boring32.win32;
import :raii;
import :error;
import :security_constants;
import :security_functions;

export namespace Boring32::Security
{
	class Token final
	{
		public:
			~Token()
			{
				Close();
			}

			Token() = default;

			Token(const Token& other)
			{
				Copy(other);
			}

			Token& operator=(const Token& other)
			{
				return Copy(other);
			}

			Token(Token&& other) noexcept
			{
				Move(other);
			}

			Token& operator=(Token&& other) noexcept
			{
				return Move(other);
			}

		public:
			Token(const Win32::DWORD desiredAccess)
			{
				m_token = GetProcessToken(Win32::GetCurrentProcess(), desiredAccess);
			}

			Token(const Win32::HANDLE processHandle, const Win32::DWORD desiredAccess)
			{
				m_token = GetProcessToken(processHandle, desiredAccess);
			}

			Token(const Win32::HANDLE token, const bool ownOrDuplicate)
			{
				if (not token)
					throw Error::Boring32Error("Token cannot be null");

				if (ownOrDuplicate)
				{
					m_token = RAII::Win32Handle(token);
					return;
				}

				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-duplicatetokenex
				const bool succeeded = Win32::DuplicateTokenEx(
					token,
					0,
					nullptr,
					Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
					Win32::TOKEN_TYPE::TokenPrimary,
					&m_token
				);
				if (not succeeded)
					throw Error::Win32Error(Win32::GetLastError(), "DuplicateTokenEx() failed");
			}
			
		public:
			void Close()
			{
				m_token = nullptr;
			}

			RAII::Win32Handle GetToken() const noexcept
			{
				return m_token;
			}

			/// <summary>
			/// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
			/// </summary>
			void AdjustPrivileges(const std::wstring& privilege, const bool enabled)
			{
				Security::AdjustPrivileges(m_token.GetHandle(), privilege, enabled);
			}

			void SetIntegrity(const Constants::GroupIntegrity integrity)
			{
				Security::SetIntegrity(m_token.GetHandle(), integrity);
			}

		private:
			Token& Copy(const Token& other)
			{
				if (&other == this)
					return *this;

				Close();
				if (other.m_token)
				{
					const bool succeeded = Win32::DuplicateTokenEx(
						other.m_token.GetHandle(),
						0,
						nullptr,
						Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
						Win32::TOKEN_TYPE::TokenPrimary,
						&m_token
					);
					if (not succeeded)
						throw Error::Win32Error(Win32::GetLastError(), "DuplicateTokenEx() failed");
				}

				return *this;
			}

			Token& Move(Token& other) noexcept
			{
				if (&other == this)
					return *this;

				Close();
				m_token = std::move(other.m_token);

				return *this;
			}

		private:
			RAII::Win32Handle m_token;
	};
}
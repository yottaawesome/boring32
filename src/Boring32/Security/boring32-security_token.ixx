export module boring32:security_token;
import std;
import boring32.win32;
import :raii;
import :error;
import :security_constants;
import :security_functions;

export namespace Boring32::Security
{
	struct Token final
	{
		Token() = default;
		Token(Token&& other) noexcept = default;
		Token& operator=(Token&& other) noexcept = default;

		Token(const Token& other)
		{
			Copy(other);
		}

		Token& operator=(const Token& other)
		{
			return Copy(other);
		}

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
			bool isPrimary = GetTokenInfo<Win32::TOKEN_INFORMATION_CLASS::TokenType, Win32::TOKEN_TYPE>(token);
			bool succeeded = Win32::DuplicateTokenEx(
				token,
				0,
				nullptr,
				isPrimary
					? Win32::SECURITY_IMPERSONATION_LEVEL::SecurityIdentification 
					: Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
				isPrimary
					? Win32::TOKEN_TYPE::TokenPrimary 
					: Win32::TOKEN_TYPE::TokenImpersonation,
				&m_token
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "DuplicateTokenEx() failed");
		}
			
		operator bool() const noexcept
		{
			return m_token != nullptr;
		}

		Token ToImpersonationToken() const
		{
			if (not m_token)
				throw Error::Boring32Error("Token cannot be null");
			Win32::HANDLE impersonationToken;
			bool succeeded = Win32::DuplicateTokenEx(
				m_token,
				0,
				nullptr,
				Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
				Win32::TOKEN_TYPE::TokenImpersonation,
				&impersonationToken
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "DuplicateTokenEx() failed");
			return { impersonationToken, true };
		}

		void Close()
		{
			m_token = nullptr;
		}

		RAII::Win32Handle GetToken() const noexcept
		{
			return m_token;
		}

		/// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
		void AdjustPrivileges(const std::wstring& privilege, const bool enabled)
		{
			Security::AdjustPrivileges(m_token.GetHandle(), privilege, enabled);
		}

		void SetIntegrity(const Constants::GroupIntegrity integrity)
		{
			Security::SetIntegrity(m_token.GetHandle(), integrity);
		}

		auto GetType() const -> Win32::TOKEN_TYPE
		{
			return GetTokenInfo<Win32::TOKEN_INFORMATION_CLASS::TokenType, Win32::TOKEN_TYPE>(m_token.GetHandle());
		}

		bool IsPrimary() const
		{
			return GetType() == Win32::TOKEN_TYPE::TokenPrimary;
		}

		bool IsImpersonation() const
		{
			return GetType() != Win32::TOKEN_TYPE::TokenPrimary;
		}

		Win32::TOKEN_STATISTICS GetStatistics() const
		{
			return GetTokenInfo<Win32::TOKEN_INFORMATION_CLASS::TokenStatistics, Win32::TOKEN_STATISTICS>(m_token.GetHandle());
		}

	private:
		template<auto VInfoType, typename TReturn>
		static auto GetTokenInfo(Win32::HANDLE token)
		{
			if (not token)
				throw Error::RuntimeError("No token to query.");

			TReturn type;
			Win32::DWORD returnLength;
			bool succeeded = Win32::GetTokenInformation(
				token,
				VInfoType,
				&type,
				sizeof(type),
				&returnLength
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "GetTokenInformation() failed.");
			return type;
		}

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

		RAII::Win32Handle m_token;
	};
}
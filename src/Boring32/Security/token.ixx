export module boring32:security.token;
import std;
import :win32;
import :raii;
import :error;
import :security.constants;
import :security.functions;
import :security.securityidentifier;

export namespace Boring32::Security
{
	struct TokenGroups
	{
		SidAndAttributes Sid;
		auto Enabled(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupEnabled;
		}
		auto EnabledByDefault(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupEnabledByDefault;
		}
		auto Integrity(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupIntegrity;
		}
		auto IntegrityEnabled(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupIntegrityEnabled;
		}
		auto LogonId(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupLogonId;
		}
		auto Mandatory(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupMandatory;
		}
		auto Owner(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupOwner;
		}
		auto Resource(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupResource;
		}
		auto UseForDenyOnly(this auto& self) noexcept -> bool
		{
			return self.Sid.Attributes & Win32::SeGroupUseForDenyOnly;
		}
	};

	class Token final
	{
	public:
		Token() = default;
		
		Token(const Token& other)
		{
			Copy(other);
		}
		auto operator=(const Token& other) -> Token&
		{
			return Copy(other);
		}
		Token(Token&& other) noexcept = default;
		auto operator=(Token&& other) noexcept -> Token& = default;

		Token(const Win32::DWORD desiredAccess)
		{
			m_token = GetProcessToken(Win32::GetCurrentProcess(), desiredAccess);
		}

		Token(Win32::HANDLE processHandle, Win32::DWORD desiredAccess)
		{
			m_token = GetProcessToken(processHandle, desiredAccess);
		}

		Token(Win32::HANDLE token, bool ownOrDuplicate)
		{
			if (not token)
				throw Error::Boring32Error{"Token cannot be null"};

			if (ownOrDuplicate)
			{
				m_token = RAII::UniqueHandle(token);
				return;
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-duplicatetokenex
			auto type = GetTokenInfo<Win32::TOKEN_TYPE>(token, Win32::TOKEN_INFORMATION_CLASS::TokenType);
			auto isPrimary = type == Win32::TOKEN_TYPE::TokenPrimary;
			auto succeeded = 
				Win32::DuplicateTokenEx(
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
				throw Error::Win32Error{Win32::GetLastError(), "DuplicateTokenEx() failed"};
		}
			
		operator bool() const noexcept
		{
			return m_token != nullptr;
		}

		auto ToImpersonationToken() const -> Token
		{
			if (not m_token)
				throw Error::Boring32Error{"Token cannot be null"};
			auto impersonationToken = Win32::HANDLE{};
			auto succeeded = Win32::DuplicateTokenEx(
				m_token,
				0,
				nullptr,
				Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
				Win32::TOKEN_TYPE::TokenImpersonation,
				&impersonationToken
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "DuplicateTokenEx() failed"};
			return { impersonationToken, true };
		}

		void Close()
		{
			m_token = nullptr;
		}

		auto GetToken() const noexcept -> Win32::HANDLE
		{
			return m_token.GetHandle();
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
			return GetTokenInfo<Win32::TOKEN_TYPE>(m_token.GetHandle(), Win32::TOKEN_INFORMATION_CLASS::TokenType);
		}

		auto IsPrimary() const -> bool
		{
			return GetType() == Win32::TOKEN_TYPE::TokenPrimary;
		}

		auto IsImpersonation() const -> bool
		{
			return GetType() != Win32::TOKEN_TYPE::TokenPrimary;
		}

		auto GetStatistics() const -> Win32::TOKEN_STATISTICS
		{
			return GetTokenInfo<Win32::TOKEN_STATISTICS>(m_token.GetHandle(), Win32::TOKEN_INFORMATION_CLASS::TokenStatistics);
		}

		auto CheckMembership(std::wstring_view sidString) const -> bool
		{
			auto sid = SecurityIdentifier{ std::wstring{ sidString } };
			return false;
		}

		auto CheckMembership(Win32::PSID sid) const -> bool
		{
			if (not m_token)
				throw Error::Boring32Error{"No token to check"};
			if (not sid)
				throw Error::Boring32Error{"Invalid sid"};
			auto isMember = Win32::BOOL{};
			if (not Win32::CheckTokenMembership(m_token.GetHandle(), sid, &isMember))
				throw Error::Win32Error{Win32::GetLastError(), "CheckTokenMembership() failed"};
			return isMember != 0;
		}

		auto GetGroups() const -> std::vector<TokenGroups>
		{
			if (not m_token)
				return {};
			auto buffer = std::vector<std::byte>{ GetTokenInfo(m_token.GetHandle(), Win32::TOKEN_INFORMATION_CLASS::TokenGroups) };
			auto groups = reinterpret_cast<Win32::TOKEN_GROUPS*>(buffer.data());
			auto tokenGroups = std::vector<TokenGroups>{};
			for (auto i = Win32::DWORD{}; i < groups->GroupCount; i++)
			{
				auto sid = Win32::SID_AND_ATTRIBUTES{ groups->Groups[i] };
				tokenGroups.push_back(TokenGroups{ SidAndAttributes{ sid.Sid, sid.Attributes } });
			}

			return tokenGroups;
		}

		auto GetUser() const -> SidAndAttributes
		{
			auto buffer = GetTokenInfo(m_token.GetHandle(), Win32::TOKEN_INFORMATION_CLASS::TokenUser);
			auto user = reinterpret_cast<Win32::TOKEN_USER*>(buffer.data());
			return {user->User.Sid, user->User.Attributes};
		}

		template<typename TReturn>
		static auto GetTokenInfo(Win32::HANDLE token, Win32::TOKEN_INFORMATION_CLASS infoType) -> TReturn
		{
			auto buffer = GetTokenInfo(token, infoType);
			return TReturn{ *reinterpret_cast<TReturn*>(buffer.data()) };
		}

		static auto GetTokenInfo(Win32::HANDLE token, Win32::TOKEN_INFORMATION_CLASS infoType)
		{
			if (not token)
				throw Error::RuntimeError{"No token to query."};

			auto returnLength = Win32::DWORD{};
			auto succeeded = 
				Win32::GetTokenInformation(
					token,
					infoType,
					nullptr,
					0,
					&returnLength
				);
			if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error{lastError, "GetTokenInformation() failed."};

			auto value = std::vector<std::byte>{ returnLength };
			succeeded = Win32::GetTokenInformation(
				token,
				infoType,
				value.data(),
				returnLength,
				&returnLength
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "GetTokenInformation() failed."};

			return value;
		}

	private:
		auto Copy(const Token& other) -> Token&
		{
			if (&other == this)
				return *this;
			Close();
			if (not other.m_token)
				return *this;

			auto succeeded = 
				Win32::DuplicateTokenEx(
					other.m_token.GetHandle(),
					0,
					nullptr,
					Win32::SECURITY_IMPERSONATION_LEVEL::SecurityImpersonation,
					Win32::TOKEN_TYPE::TokenPrimary,
					&m_token
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "DuplicateTokenEx() failed"};

			return *this;
		}

		RAII::UniqueHandle m_token;
	};
}
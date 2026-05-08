export module boring32:security.securityidentifier;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid
	class SecurityIdentifier final
	{
	public:
		SecurityIdentifier() = default;

		SecurityIdentifier(
			const Win32::SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
			std::uint8_t subAuthorities
		)
		{
			Create(pIdentifierAuthority, subAuthorities);
		}

		SecurityIdentifier(PSID sid)
		{
			Create(sid);
		}

		SecurityIdentifier(std::wstring_view sidString)
			: SecurityIdentifier()
		{
			Create(sidString);
		}

		auto ToSidString() const -> std::wstring
		{
			auto string = Win32::LPWSTR{};
			// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
			if (not Win32::ConvertSidToStringSidW(reinterpret_cast<PSID>(const_cast<std::byte*>(m_sid.data())), &string))
				throw Error::Win32Error{Win32::GetLastError(), "ConvertSidToStringSidW() failed"};
			auto ptr = RAII::LocalHeapUniquePtr<wchar_t>{string};
			return string;
		}

		auto GetSid() const noexcept -> Win32::PSID
		{
			return reinterpret_cast<PSID>(const_cast<std::byte*>(m_sid.data()));
		}

		auto GetSubAuthorityCount() const -> std::uint8_t
		{
			return GetSubAuthorityCount(GetSid());
		}

		static auto GetSubAuthorityCount(Win32::PSID sid) -> std::uint8_t
		{
			if (not Win32::IsValidSid(sid))
				throw Error::Boring32Error{"Invalid SID"};

			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
			auto authorityCount = Win32::PUCHAR{ Win32::GetSidSubAuthorityCount(sid) };
			return static_cast<std::uint8_t>(*authorityCount);
		}

		auto GetIdentifierAuthority() const -> Win32::SID_IDENTIFIER_AUTHORITY
		{
			return GetIdentifierAuthority(GetSid());
		}

		static auto GetIdentifierAuthority(Win32::PSID sid) -> Win32::SID_IDENTIFIER_AUTHORITY
		{
			if (not sid)
				throw Error::Boring32Error{"No valid SID"};
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
			if (auto identifier = Win32::PSID_IDENTIFIER_AUTHORITY{Win32::GetSidIdentifierAuthority(sid)})
				return *identifier;

			throw Error::Win32Error{Win32::GetLastError(), "GetSidIdentifierAuthority() failed"};
		}

		auto GetSubAuthority(const Win32::DWORD index) const -> Win32::DWORD
		{
			if (m_sid.empty())
				throw Error::Boring32Error{"No valid SID"};
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
			if (auto returnVal = Win32::PDWORD{Win32::GetSidSubAuthority(GetSid(), index)})
				return *returnVal;

			throw Error::Win32Error{Win32::GetLastError(), "GetSidSubAuthority() failed"};
		}

		static auto GetSubAuthority(PSID sid, Win32::DWORD index) -> Win32::DWORD
		{
			if (not sid)
				throw Error::Boring32Error{"No valid SID"};
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
			if (auto returnVal = Win32::PDWORD{Win32::GetSidSubAuthority(sid, index)})
				return *returnVal;

			throw Error::Win32Error{Win32::GetLastError(), "GetSidSubAuthority() failed"};
		}

		auto GetAllSubAuthorities() const -> std::vector<Win32::DWORD>
		{
			return GetAllSubAuthorities(GetSid());
		}

		static auto GetAllSubAuthorities(PSID sid) -> std::vector<Win32::DWORD>
		{
			if (not sid)
				return {};

			auto returnVal = std::vector<Win32::DWORD>{};
			for (auto i = std::uint8_t{}, count = GetSubAuthorityCount(sid); i < count; i++)
				returnVal.push_back(GetSubAuthority(sid, i));
			return returnVal;
		}

		auto HasValue() const noexcept -> bool
		{
			return not m_sid.empty();
		}

		auto LookupName() const -> std::wstring
		{
			auto count = Win32::DWORD{512};
			auto domainCount = Win32::DWORD{256};
			auto nameUse = Win32::SID_NAME_USE{};
			auto name = std::wstring(count, '\0');
			auto domain = std::wstring(domainCount, '\0');
			auto domainCCh = Win32::DWORD{0};
			auto success = 
				Win32::LookupAccountSidW(
					nullptr,
					GetSid(),
					name.data(),
					&count,
					domain.data(),
					&domainCount,
					&nameUse
				);
			if (not success)
				if (auto error = Win32::GetLastError(); error != Win32::ErrorCodes::NoneMapped)
					throw Error::Win32Error{ error, "LookupAccountSidW() failed" };
			name.resize(count);
			domain.resize(domainCount);
			return domain.empty() ? name : std::format(L"{}\\{}", domain, name);
		}

	private:
		void Create(std::wstring_view sidString)
		{
			if (sidString.empty())
				throw Error::Boring32Error{"sidString cannot be empty"};
			// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
			auto sid = Win32::PSID{};
			if (not Win32::ConvertStringSidToSidW(sidString.data(), &sid))
				throw Error::Win32Error{Win32::GetLastError(), "ConvertStringSidToSidW() failed"};
			auto ptr = RAII::LocalHeapUniquePtr<void>{ sid };
			Create(sid);
		}

		void Create(PSID sid)
		{
			if (not sid)
				throw Error::Boring32Error{"Invalid sid"};
			m_sid.resize(Win32::GetLengthSid(sid));
			auto succeeded = 
				Win32::CopySid(static_cast<Win32::DWORD>(m_sid.size()), m_sid.data(), sid);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "CopySid() failed"};
		}

		void Create(
			const Win32::SID_IDENTIFIER_AUTHORITY& identifierAuthority,
			std::uint8_t subAuthorities
		)
		{
			m_sid.resize(Win32::GetSidLengthRequired(subAuthorities));

			auto succeeded = Win32::InitializeSid(
				GetSid(), 
				const_cast<Win32::PSID_IDENTIFIER_AUTHORITY>(&identifierAuthority), 
				subAuthorities
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "Failed to initialise SID"};
		}
			
		std::vector<std::byte> m_sid;
	};

	struct SidAndAttributes final
	{
		SecurityIdentifier Sid;
		Win32::DWORD Attributes = 0;
	};
}
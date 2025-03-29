export module boring32:security_securityidentifier;
import std;
import boring32.win32;
import :error;
import :raii;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid
	struct SecurityIdentifier final
	{
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

		SecurityIdentifier(const std::wstring& sidString)
			: SecurityIdentifier()
		{
			Create(sidString);
		}

		operator std::wstring() const
		{
			Win32::LPWSTR string = nullptr;
			// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
			if (not Win32::ConvertSidToStringSidW(reinterpret_cast<PSID>(const_cast<std::byte*>(m_sid.data())), &string))
				throw Error::Win32Error(Win32::GetLastError(), "ConvertSidToStringSidW() failed");
			RAII::LocalHeapUniquePtr<wchar_t> ptr(string);
			return string;
		}

		Win32::PSID GetSid() const noexcept
		{
			return reinterpret_cast<PSID>(const_cast<std::byte*>(m_sid.data()));
		}

		std::uint8_t GetSubAuthorityCount() const
		{
			return GetSubAuthorityCount(GetSid());
		}

		static std::uint8_t GetSubAuthorityCount(Win32::PSID sid)
		{
			if (not Win32::IsValidSid(sid))
				throw Error::Boring32Error("Invalid SID");

			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
			Win32::PUCHAR authorityCount = Win32::GetSidSubAuthorityCount(sid);
			return static_cast<std::uint8_t>(*authorityCount);
		}

		Win32::SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const
		{
			return GetIdentifierAuthority(GetSid());
		}

		static Win32::SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority(Win32::PSID sid)
		{
			if (not sid)
				throw Error::Boring32Error("No valid SID");
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
			if (Win32::PSID_IDENTIFIER_AUTHORITY identifier = Win32::GetSidIdentifierAuthority(sid))
				return *identifier;

			throw Error::Win32Error(Win32::GetLastError(), "GetSidIdentifierAuthority() failed");
		}

		Win32::DWORD GetSubAuthority(const Win32::DWORD index) const
		{
			if (m_sid.empty())
				throw Error::Boring32Error("No valid SID");
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
			if (Win32::PDWORD returnVal = Win32::GetSidSubAuthority(GetSid(), index))
				return *returnVal;

			throw Error::Win32Error(Win32::GetLastError(),"GetSidSubAuthority() failed");
		}

		static Win32::DWORD GetSubAuthority(PSID sid, Win32::DWORD index)
		{
			if (not sid)
				throw Error::Boring32Error("No valid SID");
			// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
			if (Win32::PDWORD returnVal = Win32::GetSidSubAuthority(sid, index))
				return *returnVal;

			throw Error::Win32Error(Win32::GetLastError(), "GetSidSubAuthority() failed");
		}

		std::vector<Win32::DWORD> GetAllSubAuthorities() const
		{
			return GetAllSubAuthorities(GetSid());
		}

		static std::vector<Win32::DWORD> GetAllSubAuthorities(PSID sid)
		{
			if (not sid)
				return {};

			std::vector<Win32::DWORD> returnVal;
			for (Win32::BYTE i = 0, count = GetSubAuthorityCount(sid); i < count; i++)
				returnVal.push_back(GetSubAuthority(sid, i));
			return returnVal;
		}

	private:
		void Create(const std::wstring& sidString)
		{
			if (sidString.empty())
				throw Error::Boring32Error("sidString cannot be empty");
			// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
			Win32::PSID sid = nullptr;
			if (not Win32::ConvertStringSidToSidW(sidString.c_str(), &sid))
				throw Error::Win32Error(Win32::GetLastError(), "ConvertStringSidToSidW() failed");
			RAII::LocalHeapUniquePtr<void> ptr(sid);
			Create(sid);
		}

		void Create(PSID sid)
		{
			if (not sid)
				throw Error::Boring32Error("Invalid sid");
			m_sid.resize(Win32::GetLengthSid(sid));
			BOOL succeeded = Win32::CopySid(
				static_cast<Win32::DWORD>(m_sid.size()),
				m_sid.data(),
				sid
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "CopySid() failed");
		}

		void Create(
			const Win32::SID_IDENTIFIER_AUTHORITY& identifierAuthority,
			std::uint8_t subAuthorities
		)
		{
			m_sid.resize(Win32::GetSidLengthRequired(subAuthorities));

			bool succeeded = Win32::InitializeSid(
				GetSid(), 
				const_cast<Win32::PSID_IDENTIFIER_AUTHORITY>(&identifierAuthority), 
				subAuthorities
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "Failed to initialise SID");
		}
			
		std::vector<std::byte> m_sid;
	};
}
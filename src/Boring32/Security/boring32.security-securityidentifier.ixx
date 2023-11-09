export module boring32.security:securityidentifier;
import <vector>;
import <string>;
import <iostream>;
import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid
	class SecurityIdentifier final
	{
		public:
			~SecurityIdentifier()
			{
				Close();
			}

			SecurityIdentifier() = default;

			SecurityIdentifier(const SecurityIdentifier& other)
				: m_sid(nullptr)
			{
				Copy(other);
			}

			SecurityIdentifier& operator=(const SecurityIdentifier& other)
			{
				Copy(other);
				return *this;
			}

			SecurityIdentifier(SecurityIdentifier&& other) noexcept
				: m_sid(nullptr)
			{
				Move(other);
			}

			SecurityIdentifier& operator=(SecurityIdentifier&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			SecurityIdentifier(
				const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<DWORD>& subAuthorities
			) : m_sid(nullptr)
			{
				Create(pIdentifierAuthority, subAuthorities);
			}

			SecurityIdentifier(const std::wstring& sidString)
				: SecurityIdentifier()
			{
				Create(sidString);
			}

		public:
			operator std::wstring() const
			{
				LPWSTR string = nullptr;
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
				const bool succeeded = ConvertSidToStringSidW(m_sid, &string);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ConvertSidToStringSidW() failed", lastError);
				}
				RAII::LocalHeapUniquePtr<wchar_t> ptr(string);
				return string;
			}

		public:
			void Close()
			{
				if (!m_sid)
					return;
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-freesid
				if (FreeSid(m_sid))
					std::wcerr << L"Failed to release SID\n";
				m_sid = nullptr;
			}

			PSID GetSid() const noexcept
			{
				return m_sid;
			}

			BYTE GetSubAuthorityCount() const
			{
				if (!m_sid)
					return 0;
				if (!IsValidSid(m_sid))
					throw Error::Boring32Error("Invalid SID");

				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
				PUCHAR authorityCount = GetSidSubAuthorityCount(m_sid);
				return static_cast<BYTE>(*authorityCount);
			}

			SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
				if (PSID_IDENTIFIER_AUTHORITY identifier = GetSidIdentifierAuthority(m_sid))
					return *identifier;

				const auto lastError = GetLastError();
				throw Error::Win32Error("GetSidIdentifierAuthority() failed", lastError);
			}

			DWORD GetSubAuthority(const DWORD index) const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
				if (PDWORD returnVal = GetSidSubAuthority(m_sid, index))
					return *returnVal;

				const auto lastError = GetLastError();
				throw Error::Win32Error("GetSidSubAuthority() failed", lastError);
			}

			std::vector<DWORD> GetAllSubAuthorities() const
			{
				if (!m_sid)
					return {};

				std::vector<DWORD> returnVal;
				for (BYTE i = 0, count = GetSubAuthorityCount(); i < count; i++)
					returnVal.push_back(GetSubAuthority(i));
				return returnVal;
			}

		private:
			void Copy(const SecurityIdentifier& other)
			{
				if (&other == this)
					return;

				Close();
				if (!other.m_sid)
					return;

				Create(other.GetIdentifierAuthority(), other.GetAllSubAuthorities());
			}

			void Move(SecurityIdentifier& other) noexcept
			{
				Close();
				if (!other.m_sid)
					return;

				m_sid = other.m_sid;
				other.m_sid = nullptr;
			}

			void Create(const std::wstring& sidString)
			{
				if (sidString.empty())
					throw Error::Boring32Error("sidString cannot be empty");
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
				if (!ConvertStringSidToSidW(sidString.c_str(), &m_sid))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ConvertStringSidToSidW() failed", lastError);
				}
			}

			void Create(
				const SID_IDENTIFIER_AUTHORITY& identifierAuthority,
				const std::vector<DWORD>& subAuthorities
			)
			{
				// What's the reason for doing this?
				std::vector<DWORD> subAuthorities2 = subAuthorities;
				if (subAuthorities2.size() != 8)
					subAuthorities2.resize(8);

				const bool succeeded = AllocateAndInitializeSid(
					&const_cast<SID_IDENTIFIER_AUTHORITY&>(identifierAuthority),
					static_cast<BYTE>(subAuthorities.size()),
					subAuthorities2[0],
					subAuthorities2[1],
					subAuthorities2[2],
					subAuthorities2[3],
					subAuthorities2[4],
					subAuthorities2[5],
					subAuthorities2[6],
					subAuthorities2[7],
					&m_sid
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("Failed to initialise SID", lastError);
				}
			}
			
		private:
			PSID m_sid = nullptr;
	};
}
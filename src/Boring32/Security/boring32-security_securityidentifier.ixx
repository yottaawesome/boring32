export module boring32:security_securityidentifier;
import boring32.shared;
import :error;
import :raii;

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
				const Win32::SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<Win32::DWORD>& subAuthorities
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
				Win32::LPWSTR string = nullptr;
				// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
				const bool succeeded = Win32::ConvertSidToStringSidW(m_sid, &string);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
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
				if (Win32::FreeSid(m_sid))
					std::wcerr << L"Failed to release SID\n";
				m_sid = nullptr;
			}

			Win32::PSID GetSid() const noexcept
			{
				return m_sid;
			}

			Win32::BYTE GetSubAuthorityCount() const
			{
				if (!m_sid)
					return 0;
				if (!Win32::IsValidSid(m_sid))
					throw Error::Boring32Error("Invalid SID");

				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
				Win32::PUCHAR authorityCount = Win32::GetSidSubAuthorityCount(m_sid);
				return static_cast<Win32::BYTE>(*authorityCount);
			}

			Win32::SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
				if (Win32::PSID_IDENTIFIER_AUTHORITY identifier = Win32::GetSidIdentifierAuthority(m_sid))
					return *identifier;

				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("GetSidIdentifierAuthority() failed", lastError);
			}

			Win32::DWORD GetSubAuthority(const Win32::DWORD index) const
			{
				if (!m_sid)
					throw Error::Boring32Error("No valid SID");
				// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
				if (Win32::PDWORD returnVal = Win32::GetSidSubAuthority(m_sid, index))
					return *returnVal;

				const auto lastError = Win32::GetLastError();
				throw Error::Win32Error("GetSidSubAuthority() failed", lastError);
			}

			std::vector<Win32::DWORD> GetAllSubAuthorities() const
			{
				if (!m_sid)
					return {};

				std::vector<Win32::DWORD> returnVal;
				for (Win32::BYTE i = 0, count = GetSubAuthorityCount(); i < count; i++)
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
				if (!Win32::ConvertStringSidToSidW(sidString.c_str(), &m_sid))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("ConvertStringSidToSidW() failed", lastError);
				}
			}

			void Create(
				const Win32::SID_IDENTIFIER_AUTHORITY& identifierAuthority,
				const std::vector<Win32::DWORD>& subAuthorities
			)
			{
				// What's the reason for doing this?
				std::vector<Win32::DWORD> subAuthorities2 = subAuthorities;
				if (subAuthorities2.size() != 8)
					subAuthorities2.resize(8);

				const bool succeeded = Win32::AllocateAndInitializeSid(
					&const_cast<Win32::SID_IDENTIFIER_AUTHORITY&>(identifierAuthority),
					static_cast<Win32::BYTE>(subAuthorities.size()),
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
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("Failed to initialise SID", lastError);
				}
			}
			
		private:
			Win32::PSID m_sid = nullptr;
	};
}
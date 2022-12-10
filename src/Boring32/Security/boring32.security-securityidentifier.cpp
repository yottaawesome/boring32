module;

#include <source_location>

module boring32.security:securityidentifier;
import boring32.error;
import boring32.raii;
import <stdexcept>;
import <iostream>;

namespace Boring32::Security
{
	SecurityIdentifier::~SecurityIdentifier()
	{
		Close();
	}

	SecurityIdentifier::SecurityIdentifier()
	:	m_sid(nullptr)
	{ }

	SecurityIdentifier::SecurityIdentifier(const SecurityIdentifier& other)
	:	m_sid(nullptr)
	{
		Copy(other);
	}

	SecurityIdentifier::SecurityIdentifier(SecurityIdentifier&& other) noexcept
	:	m_sid(nullptr)
	{
		Move(other);
	}

	SecurityIdentifier::SecurityIdentifier(
		const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
		const std::vector<DWORD>& subAuthorities
	)
	:	m_sid(nullptr)
	{
		Create(pIdentifierAuthority, subAuthorities);
	}

	SecurityIdentifier::SecurityIdentifier(const std::wstring& sidString)
		: SecurityIdentifier()
	{
		Create(sidString);
	}

	void SecurityIdentifier::Close()
	{
		if (!m_sid)
			return;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-freesid
		if (FreeSid(m_sid))
			std::wcerr << TEXT(__FUNCSIG__) L": failed to release SID" << std::endl;
		m_sid = nullptr;
	}

	PSID SecurityIdentifier::GetSid() const noexcept
	{
		return m_sid;
	}
	
	BYTE SecurityIdentifier::GetSubAuthorityCount() const
	{
		if (!m_sid)
			return 0;
		if (!IsValidSid(m_sid))
			throw Error::Boring32Error("Invalid SID");

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
		PUCHAR authorityCount = GetSidSubAuthorityCount(m_sid);
		return (BYTE)*authorityCount;
	}

	SID_IDENTIFIER_AUTHORITY SecurityIdentifier::GetIdentifierAuthority() const
	{
		if (!m_sid)
			throw Error::Boring32Error("No valid SID");
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
		if (PSID_IDENTIFIER_AUTHORITY identifier = GetSidIdentifierAuthority(m_sid))
			return *identifier;
		throw Error::Win32Error("GetSidIdentifierAuthority() failed", GetLastError());
	}
	
	DWORD SecurityIdentifier::GetSubAuthority(const DWORD index) const
	{
		if (!m_sid)
			throw Error::Boring32Error("No valid SID");
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
		if (PDWORD returnVal = GetSidSubAuthority(m_sid, index))
			return *returnVal;
		throw Error::Win32Error("GetSidSubAuthority() failed", GetLastError());
	}

	void SecurityIdentifier::operator=(const SecurityIdentifier& other)
	{
		Copy(other);
	}

	void SecurityIdentifier::operator=(SecurityIdentifier&& other) noexcept
	{
		Move(other);
	}

	SecurityIdentifier::operator std::wstring() const
	{
		LPWSTR string = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
		const bool succeeded = ConvertSidToStringSidW(m_sid, &string);
		if (!succeeded)
			throw Error::Win32Error("ConvertSidToStringSidW() failed", GetLastError());
		RAII::LocalHeapUniquePtr<wchar_t> ptr(string);
		return string;
	}

	void SecurityIdentifier::Copy(const SecurityIdentifier& other)
	{
		if (&other == this)
			return;

		Close();
		if (!other.m_sid)
			return;

		Create(other.GetIdentifierAuthority(), other.GetAllSubAuthorities());
	}

	void SecurityIdentifier::Move(SecurityIdentifier& other) noexcept
	{
		Close();
		if (!other.m_sid)
			return;

		m_sid = other.m_sid;
		other.m_sid = nullptr;
	}

	void SecurityIdentifier::Create(const SID_IDENTIFIER_AUTHORITY& identifierAuthority, const std::vector<DWORD>& subAuthorities)
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
			throw Error::Win32Error("failed to initialise SID", GetLastError());
	}

	void SecurityIdentifier::Create(const std::wstring& sidString)
	{
		if (sidString.empty())
			throw Error::Boring32Error("sidString cannot be empty");
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		if (!ConvertStringSidToSidW(sidString.c_str(), &m_sid))
			throw Error::Win32Error("ConvertStringSidToSidW() failed", GetLastError());
	}
	
	std::vector<DWORD> SecurityIdentifier::GetAllSubAuthorities() const
	{
		if (!m_sid)
			return {};

		std::vector<DWORD> returnVal;
		for (BYTE i = 0, count = GetSubAuthorityCount(); i < count; i++)
			returnVal.push_back(GetSubAuthority(i));
		return returnVal;
	}
}

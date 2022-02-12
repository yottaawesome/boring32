module;

#include <vector>
#include <stdexcept>
#include <source_location>
#include <iostream>
#include <Windows.h>
#include <sddl.h>

module boring32.security.sid;
import boring32.error.win32error;
import boring32.raii.uniqueptrs;

namespace Boring32::Security
{
	Sid::~Sid()
	{
		Close();
	}

	Sid::Sid()
	:	m_sid(nullptr)
	{ }

	Sid::Sid(const Sid& other)
	:	m_sid(nullptr)
	{
		Copy(other);
	}

	Sid::Sid(Sid&& other) noexcept
	:	m_sid(nullptr)
	{
		Move(other);
	}

	Sid::Sid(
		const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
		const std::vector<DWORD>& subAuthorities
	)
	:	m_sid(nullptr)
	{
		Create(pIdentifierAuthority, subAuthorities);
	}

	Sid::Sid(const std::wstring& sidString)
		: Sid()
	{
		Create(sidString);
	}

	void Sid::Close()
	{
		if (!m_sid)
			return;
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-freesid
		if (FreeSid(m_sid))
			std::wcerr << TEXT(__FUNCSIG__) L": failed to release SID" << std::endl;
		m_sid = nullptr;
	}

	PSID Sid::GetSid() const noexcept
	{
		return m_sid;
	}
	
	BYTE Sid::GetSubAuthorityCount() const
	{
		if (!m_sid)
			return 0;
		if (!IsValidSid(m_sid))
			throw std::runtime_error(__FUNCSIG__ ": invalid SID");

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthoritycount
		PUCHAR authorityCount = GetSidSubAuthorityCount(m_sid);
		return (BYTE)*authorityCount;
	}

	SID_IDENTIFIER_AUTHORITY Sid::GetIdentifierAuthority() const
	{
		if (!m_sid)
			throw std::runtime_error(__FUNCSIG__": no valid SID");
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsididentifierauthority
		if (PSID_IDENTIFIER_AUTHORITY identifier = GetSidIdentifierAuthority(m_sid))
			return *identifier;
		throw Error::Win32Error(std::source_location::current(), "GetSidIdentifierAuthority() failed", GetLastError());
	}
	
	DWORD Sid::GetSubAuthority(const DWORD index) const
	{
		if (!m_sid)
			throw std::runtime_error(__FUNCSIG__": no valid SID");
		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-getsidsubauthority
		if (PDWORD returnVal = GetSidSubAuthority(m_sid, index))
			return *returnVal;
		throw Error::Win32Error(std::source_location::current(), "GetSidSubAuthority() failed", GetLastError());
	}

	void Sid::operator=(const Sid& other)
	{
		Copy(other);
	}

	void Sid::operator=(Sid&& other) noexcept
	{
		Move(other);
	}

	Sid::operator std::wstring() const
	{
		LPWSTR string = nullptr;
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertsidtostringsidw
		const bool succeeded = ConvertSidToStringSidW(m_sid, &string);
		if (!succeeded)
			throw Error::Win32Error(std::source_location::current(), "ConvertSidToStringSidW() failed", GetLastError());
		Raii::LocalHeapUniquePtr ptr(string);
		return string;
	}

	void Sid::Copy(const Sid& other)
	{
		if (&other == this)
			return;

		Close();
		if (!other.m_sid)
			return;

		Create(other.GetIdentifierAuthority(), other.GetAllSubAuthorities());
	}

	void Sid::Move(Sid& other) noexcept
	{
		Close();
		if (!other.m_sid)
			return;

		m_sid = other.m_sid;
		other.m_sid = nullptr;
	}

	void Sid::Create(const SID_IDENTIFIER_AUTHORITY& identifierAuthority, const std::vector<DWORD>& subAuthorities)
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
			throw Error::Win32Error(std::source_location::current(), "failed to initialise SID", GetLastError());
	}

	void Sid::Create(const std::wstring sidString)
	{
		if (sidString.empty())
			throw std::invalid_argument(__FUNCSIG__": sidString cannot be empty");
		// https://docs.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsidtosidw
		if (!ConvertStringSidToSidW(sidString.c_str(), &m_sid))
			throw Error::Win32Error(std::source_location::current(), "ConvertStringSidToSidW() failed", GetLastError());
	}
	
	std::vector<DWORD> Sid::GetAllSubAuthorities() const
	{
		if (!m_sid)
			return {};

		std::vector<DWORD> returnVal;
		for (BYTE i = 0, count = GetSubAuthorityCount(); i < count; i++)
			returnVal.push_back(GetSubAuthority(i));
		return returnVal;
	}
}

module;

#include <vector>
#include <stdexcept>
#include <iostream>
#include <Windows.h>

module boring32.security.sid;
import boring32.error.win32error;

namespace Boring32::Security
{
	Sid::~Sid()
	{
		Close();
	}

	Sid::Sid()
	:	m_sid(nullptr),
		m_pIdentifierAuthority{}
	{ }

	Sid::Sid(const Sid& other)
	:	m_sid(nullptr),
		m_pIdentifierAuthority{}
	{
		Copy(other);
	}

	Sid::Sid(Sid&& other) noexcept
	:	m_sid(nullptr),
		m_pIdentifierAuthority{}
	{
		Move(other);
	}

	Sid::Sid(
		const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
		const std::vector<DWORD>& subAuthorities
	)
	:	m_sid(nullptr),
		m_pIdentifierAuthority(pIdentifierAuthority),
		m_subAuthorities(subAuthorities)
	{
		Create();
	}

	void Sid::Close()
	{
		if (m_sid)
		{
			if (FreeSid(m_sid) != nullptr)
				std::wcerr << L"Failed to release SID" << std::endl;
			m_sid = nullptr;
			m_subAuthorities.clear();
		}
	}

	PSID Sid::GetSid() const noexcept
	{
		return m_sid;
	}
	
	BYTE Sid::GetSubAuthorityCount() const
	{
		if (m_sid == nullptr)
			return 0;
		if (IsValidSid(m_sid) == false)
			throw std::runtime_error(__FUNCSIG__ ": invalid SID");

		PUCHAR authorityCount = GetSidSubAuthorityCount(m_sid);
		return (BYTE)*authorityCount;
	}

	void Sid::operator=(const Sid& other)
	{
		Copy(other);
	}

	void Sid::operator=(Sid&& other) noexcept
	{
		Move(other);
	}

	void Sid::Copy(const Sid& other)
	{
		if (&other == this)
			return;

		Close();
		if (other.m_sid == nullptr)
			return;

		m_pIdentifierAuthority = other.m_pIdentifierAuthority;
		m_subAuthorities = other.m_subAuthorities;

		Create();
	}

	void Sid::Move(Sid& other) noexcept
	{
		Close();
		if (other.m_sid == nullptr)
			return;

		m_sid = other.m_sid;
		m_subAuthorities = other.m_subAuthorities;
	}

	void Sid::Create()
	{
		std::vector<DWORD> subAuthorities2
			= m_subAuthorities;
		if (subAuthorities2.size() != 8)
			subAuthorities2.resize(8);

		const bool isInitialized = AllocateAndInitializeSid(
			&m_pIdentifierAuthority,
			(BYTE)m_subAuthorities.size(),
			subAuthorities2[0],
			subAuthorities2[1],
			subAuthorities2[2],
			subAuthorities2[3],
			subAuthorities2[4],
			subAuthorities2[5],
			subAuthorities2[6],
			subAuthorities2[7],
			(PSID*)&m_sid
		);
		if (isInitialized == false)
			throw Error::Win32Error(__FUNCSIG__ ": failed to initialise SID", GetLastError());
	}
}

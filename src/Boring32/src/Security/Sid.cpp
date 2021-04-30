#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Security/Sid.hpp"

namespace Boring32::Security
{
	Sid::~Sid()
	{
		Close();
	}

	Sid::Sid()
	:	m_sid(nullptr),
		m_pIdentifierAuthority{},
		m_nSubAuthorityCount(0),
		m_nSubAuthority0(0),
		m_nSubAuthority1(0),
		m_nSubAuthority2(0),
		m_nSubAuthority3(0),
		m_nSubAuthority4(0),
		m_nSubAuthority5(0),
		m_nSubAuthority6(0),
		m_nSubAuthority7(0)
	{ }

	Sid::Sid(const Sid& other)
	:	m_sid(nullptr),
		m_pIdentifierAuthority{},
		m_nSubAuthorityCount(0),
		m_nSubAuthority0(0),
		m_nSubAuthority1(0),
		m_nSubAuthority2(0),
		m_nSubAuthority3(0),
		m_nSubAuthority4(0),
		m_nSubAuthority5(0),
		m_nSubAuthority6(0),
		m_nSubAuthority7(0)
	{
		Copy(other);
	}

	Sid::Sid(Sid&& other) noexcept
	:	m_sid(nullptr),
		m_pIdentifierAuthority{},
		m_nSubAuthorityCount(0),
		m_nSubAuthority0(0),
		m_nSubAuthority1(0),
		m_nSubAuthority2(0),
		m_nSubAuthority3(0),
		m_nSubAuthority4(0),
		m_nSubAuthority5(0),
		m_nSubAuthority6(0),
		m_nSubAuthority7(0)
	{
		Move(other);
	}

	Sid::Sid(
		const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
		BYTE                      nSubAuthorityCount,
		DWORD                     nSubAuthority0,
		DWORD                     nSubAuthority1,
		DWORD                     nSubAuthority2,
		DWORD                     nSubAuthority3,
		DWORD                     nSubAuthority4,
		DWORD                     nSubAuthority5,
		DWORD                     nSubAuthority6,
		DWORD                     nSubAuthority7
	)
	:	m_sid(nullptr),
		m_pIdentifierAuthority(pIdentifierAuthority),
		m_nSubAuthorityCount(nSubAuthorityCount),
		m_nSubAuthority0(nSubAuthority0),
		m_nSubAuthority1(nSubAuthority1),
		m_nSubAuthority2(nSubAuthority2),
		m_nSubAuthority3(nSubAuthority3),
		m_nSubAuthority4(nSubAuthority4),
		m_nSubAuthority5(nSubAuthority5),
		m_nSubAuthority6(nSubAuthority6),
		m_nSubAuthority7(nSubAuthority7)
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
			m_nSubAuthorityCount = 0;
			m_nSubAuthority0 = 0;
			m_nSubAuthority1 = 0;
			m_nSubAuthority2 = 0;
			m_nSubAuthority3 = 0;
			m_nSubAuthority4 = 0;
			m_nSubAuthority5 = 0;
			m_nSubAuthority6 = 0;
			m_nSubAuthority7 = 0;
		}
	}

	PSID Sid::GetSid()
	{
		return m_sid;
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
		m_nSubAuthorityCount = other.m_nSubAuthorityCount;
		m_nSubAuthority0 = other.m_nSubAuthority0;
		m_nSubAuthority1 = other.m_nSubAuthority1;
		m_nSubAuthority2 = other.m_nSubAuthority2;
		m_nSubAuthority3 = other.m_nSubAuthority3;
		m_nSubAuthority4 = other.m_nSubAuthority4;
		m_nSubAuthority5 = other.m_nSubAuthority5;
		m_nSubAuthority6 = other.m_nSubAuthority6;
		m_nSubAuthority7 = other.m_nSubAuthority7;

		Create();
	}

	void Sid::Move(Sid& other) noexcept
	{
		Close();
		if (other.m_sid == nullptr)
			return;

		m_sid = other.m_sid;
		m_nSubAuthorityCount = other.m_nSubAuthorityCount;
		m_nSubAuthority0 = other.m_nSubAuthority0;
		m_nSubAuthority1 = other.m_nSubAuthority1;
		m_nSubAuthority2 = other.m_nSubAuthority2;
		m_nSubAuthority3 = other.m_nSubAuthority3;
		m_nSubAuthority4 = other.m_nSubAuthority4;
		m_nSubAuthority5 = other.m_nSubAuthority5;
		m_nSubAuthority6 = other.m_nSubAuthority6;
		m_nSubAuthority7 = other.m_nSubAuthority7;
	}

	void Sid::Create()
	{
		const bool isInitialized = AllocateAndInitializeSid(
			&m_pIdentifierAuthority,
			m_nSubAuthorityCount,
			m_nSubAuthority0,
			m_nSubAuthority1,
			m_nSubAuthority2,
			m_nSubAuthority3,
			m_nSubAuthority4,
			m_nSubAuthority5,
			m_nSubAuthority6,
			m_nSubAuthority7,
			(PSID*)&m_sid
		);
		if (isInitialized == false)
			throw Error::Win32Error(__FUNCSIG__ ": failed to initialise SID", GetLastError());
	}
}

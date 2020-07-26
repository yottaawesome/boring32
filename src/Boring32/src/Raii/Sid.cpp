#include "pch.hpp"
#include <stdexcept>
#include "include/Raii/Sid.hpp"

namespace Boring32::Raii
{
	Sid::~Sid()
	{
		FreeSid(m_sidBuffer);
	}

	Sid::Sid(
		SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
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
		: m_sidBuffer{ 0 },
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
		bool succeeded = AllocateAndInitializeSid(
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
			(PSID*)&m_sidBuffer
		);
		if (succeeded == false)
			throw std::runtime_error("Raii::Sid -> Failed to initialise SID");
	}

	PSID Sid::GetSid()
	{
		return m_sidBuffer;
	}

	void Sid::Copy(const Sid& other)
	{

	}

	void Sid::Move(Sid&& other) noexcept
	{

	}
}

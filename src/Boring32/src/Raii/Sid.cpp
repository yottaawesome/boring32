#include "pch.hpp"
#include <stdexcept>
#include "include/Raii/Sid.hpp"

namespace Boring32::Raii
{
	Sid::~Sid()
	{
		FreeSid(pAdminSID);
	}

	Sid::Sid(
		PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
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
		: sidBuffer{ 0 }
	{
		SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
		bool succeeded = AllocateAndInitializeSid(
			&SIDAuth,
			2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0,
			0,
			0,
			0,
			0,
			0,
			&pAdminSID);
		if (succeeded == false)
			throw std::runtime_error("Raii::Sid -> Failed to initialise SID");
	}

	PSID Sid::GetSid()
	{
		return pAdminSID;
	}
}

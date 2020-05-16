#pragma once
#include <Windows.h>

namespace Boring32::Raii
{
	class Sid
	{
		public:
			Sid(
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
			);
			virtual ~Sid();
			virtual PSID GetSid();

		protected:
			BYTE sidBuffer[256];
			PSID pAdminSID = (PSID)sidBuffer;
	};
}
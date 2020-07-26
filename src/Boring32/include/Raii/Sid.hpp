#pragma once
#include <Windows.h>

namespace Boring32::Raii
{
	class Sid
	{
		public:
			Sid(
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
			);
			virtual ~Sid();
			virtual PSID GetSid();

			Sid(const Sid&) = delete;
			void operator=(const Sid&) = delete;
			Sid(Sid&&) = delete;
			void operator=(Sid&&) = delete;

		protected:
			virtual void Copy(const Sid& other);
			virtual void Move(Sid&& other) noexcept;

		protected:
			BYTE m_sidBuffer[256];
			SID_IDENTIFIER_AUTHORITY m_pIdentifierAuthority;
			BYTE                      m_nSubAuthorityCount;
			DWORD                     m_nSubAuthority0;
			DWORD                     m_nSubAuthority1;
			DWORD                     m_nSubAuthority2;
			DWORD                     m_nSubAuthority3;
			DWORD                     m_nSubAuthority4;
			DWORD                     m_nSubAuthority5;
			DWORD                     m_nSubAuthority6;
			DWORD                     m_nSubAuthority7;
	};
}
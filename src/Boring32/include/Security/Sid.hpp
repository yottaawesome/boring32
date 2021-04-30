#pragma once
#include <vector>
#include <Windows.h>

namespace Boring32::Security
{
	class Sid
	{
		public:
			virtual ~Sid();
			Sid();
			Sid(
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
			);

			Sid(const Sid& other);
			Sid(Sid&& other) noexcept;

		public:
			void operator=(const Sid& other);
			void operator=(Sid&& other) noexcept;

		public:
			virtual void Close();
			virtual PSID GetSid();

		protected:
			virtual void Copy(const Sid& other);
			virtual void Move(Sid& other) noexcept;
			virtual void Create();

		protected:
			PSID m_sid;
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
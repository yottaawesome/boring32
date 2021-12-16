module;

#include <vector>
#include <Windows.h>

export module boring32.security.sid;

export namespace Boring32::Security
{
	class Sid
	{
		public:
			virtual ~Sid();
			Sid();
			Sid(
				const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<DWORD>& subAuthorities
			);

			Sid(const Sid& other);
			Sid(Sid&& other) noexcept;

		public:
			virtual void operator=(const Sid& other);
			virtual void operator=(Sid&& other) noexcept;

		public:
			virtual void Close();
			virtual PSID GetSid();
			virtual BYTE GetSubAuthorityCount() const;

		protected:
			virtual void Copy(const Sid& other);
			virtual void Move(Sid& other) noexcept;
			virtual void Create();

		protected:
			PSID m_sid;
			SID_IDENTIFIER_AUTHORITY m_pIdentifierAuthority;
			std::vector<DWORD> m_subAuthorities;
	};
}
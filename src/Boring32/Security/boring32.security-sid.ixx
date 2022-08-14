module;

#include <vector>
#include <string>
#include <Windows.h>

export module boring32.security:sid;

export namespace Boring32::Security
{
	class Sid
	{
		public:
			virtual ~Sid();
			Sid();
			Sid(const Sid& other);
			Sid(Sid&& other) noexcept;
			Sid(
				const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<DWORD>& subAuthorities
			);
			Sid(const std::wstring& sidString);

		public:
			virtual void operator=(const Sid& other);
			virtual void operator=(Sid&& other) noexcept;
			virtual operator std::wstring() const;

		public:
			virtual void Close();
			virtual PSID GetSid() const noexcept;
			virtual BYTE GetSubAuthorityCount() const;
			virtual SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const;
			virtual DWORD GetSubAuthority(const DWORD index) const;
			virtual std::vector<DWORD> GetAllSubAuthorities() const;

		protected:
			virtual void Copy(const Sid& other);
			virtual void Move(Sid& other) noexcept;
			virtual void Create(const std::wstring& sidString);
			virtual void Create(
				const SID_IDENTIFIER_AUTHORITY& identifierAuthority,
				const std::vector<DWORD>& subAuthorities
			);

		protected:
			PSID m_sid;
	};
}
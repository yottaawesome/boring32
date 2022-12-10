module;

#include <vector>
#include <string>

export module boring32.security:securityidentifier;
import <win32.hpp>;

export namespace Boring32::Security
{
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-sid
	class SecurityIdentifier
	{
		public:
			virtual ~SecurityIdentifier();
			SecurityIdentifier();
			SecurityIdentifier(const SecurityIdentifier& other);
			SecurityIdentifier(SecurityIdentifier&& other) noexcept;
			SecurityIdentifier(
				const SID_IDENTIFIER_AUTHORITY& pIdentifierAuthority,
				const std::vector<DWORD>& subAuthorities
			);
			SecurityIdentifier(const std::wstring& sidString);

		public:
			virtual void operator=(const SecurityIdentifier& other);
			virtual void operator=(SecurityIdentifier&& other) noexcept;
			virtual operator std::wstring() const;

		public:
			virtual void Close();
			virtual PSID GetSid() const noexcept;
			virtual BYTE GetSubAuthorityCount() const;
			virtual SID_IDENTIFIER_AUTHORITY GetIdentifierAuthority() const;
			virtual DWORD GetSubAuthority(const DWORD index) const;
			virtual std::vector<DWORD> GetAllSubAuthorities() const;

		protected:
			virtual void Copy(const SecurityIdentifier& other);
			virtual void Move(SecurityIdentifier& other) noexcept;
			virtual void Create(const std::wstring& sidString);
			virtual void Create(
				const SID_IDENTIFIER_AUTHORITY& identifierAuthority,
				const std::vector<DWORD>& subAuthorities
			);

		protected:
			PSID m_sid;
	};
}
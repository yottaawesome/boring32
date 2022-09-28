module;

#include <memory>
#include <string>
#include <Windows.h>
#include <Ntsecapi.h>

export module boring32.security:policy;

namespace Boring32::Security
{
	struct LSAHandleDeleter final
	{
		void operator()(LSA_HANDLE handle) const noexcept
		{
			LsaClose(handle);
		}
	};
	using LSAHandleUniquePtr = std::unique_ptr<std::remove_pointer<LSA_HANDLE>::type, LSAHandleDeleter>;
	using LSAHandleSharedPtr = std::shared_ptr<std::remove_pointer<LSA_HANDLE>::type>;
	LSAHandleSharedPtr CreateLSASharedPtr(LSA_HANDLE handle)
	{
		return LSAHandleSharedPtr(handle, LsaClose);
	}

	// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/opening-a-policy-object-handle
	export class Policy
	{
		public:
			virtual ~Policy();
			// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/policy-object-access-rights
			Policy(const ACCESS_MASK desiredAccess);

		public:
			virtual void Close();
			// Policy handle requires POLICY_LOOKUP_NAMES, and optionally POLICY_CREATE_ACCOUNT.
			virtual void AddAccountPrivilege(
				const PSID accountSid,
				const std::wstring& privilege
			);
			// Policy handle requires POLICY_LOOKUP_NAMES.
			virtual void RemoveAccountPrivilege(
				const PSID accountSid,
				const std::wstring& privilege
			);

		private:
			LSAHandleUniquePtr m_handle;
	};
}

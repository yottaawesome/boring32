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
			Policy(const ACCESS_MASK desiredAccess);

		public:
			virtual void Close();
			virtual void SetPrivilege(
				const PSID accountSid,
				const std::wstring& privilege,
				const bool enabled
			);

		private:
			LSAHandleUniquePtr m_handle;
	};
}

module;

#include <source_location>;

export module boring32.security:policy;
// https://www.mschaef.com/windows_h_is_wierd
//#define WIN32_NO_STATUS
//#include <Windows.h>
//#undef WIN32_NO_STATUS
//#include <ntstatus.h>
//#include <winnt.h>
//#include <Ntsecapi.h>
import <string>;
import <memory>;
import <win32.hpp>;
import boring32.error;
import :lsaunicodestring;

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

	// https://blog.katastros.com/a?ID=00750-8d94e2aa-ea28-4faf-b67d-57642f88b0bb
	inline bool NT_SUCCESS(const NTSTATUS status) noexcept
	{
		return status >= 0;
	}

	inline bool NT_ERROR(const NTSTATUS status) noexcept
	{
		return status < 0;
	}

	// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/opening-a-policy-object-handle
	export class Policy
	{
		public:
			virtual ~Policy() = default;
			// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/policy-object-access-rights
			Policy(const ACCESS_MASK desiredAccess)
			{
				LSA_OBJECT_ATTRIBUTES obj{ 0 };
				LSA_HANDLE handle;
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaopenpolicy
				// See also https://learn.microsoft.com/en-us/windows/win32/secmgmt/opening-a-policy-object-handle
				const NTSTATUS status = LsaOpenPolicy(
					nullptr,
					&obj,
					desiredAccess,
					&handle
				);
				if (NT_ERROR(status))
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
					const ULONG win32Error = LsaNtStatusToWinError(status);
					throw Error::Win32Error("LsaOpenPolicy() failed", win32Error);
				}
				m_handle = LSAHandleUniquePtr(handle);
			}

		public:
			virtual void Close()
			{
				m_handle.reset();
			}

			// Policy handle requires POLICY_LOOKUP_NAMES, and optionally POLICY_CREATE_ACCOUNT.
			virtual void AddAccountPrivilege(
				const PSID accountSid,
				const std::wstring& privilege
			)
			{
				if (!accountSid)
					throw Error::Boring32Error("accountSid cannot be null");
				if (privilege.empty())
					throw Error::Boring32Error("Invalid empty privilege");

				// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
				// See also the return values for LSA: https://learn.microsoft.com/en-us/windows/win32/secmgmt/management-return-values
				LSAUnicodeString lsaPrivStr(privilege);
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaaddaccountrights
				const NTSTATUS status = LsaAddAccountRights(
					m_handle.get(), // open policy handle
					accountSid,     // target SID
					lsaPrivStr.Get(),		// privileges
					1               // privilege count
				);
				if (NT_ERROR(status))
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
					const ULONG win32Error = LsaNtStatusToWinError(status);
					throw Error::Win32Error("LsaAddAccountRights() failed", win32Error);
				}
			}

			// Policy handle requires POLICY_LOOKUP_NAMES.
			virtual void RemoveAccountPrivilege(
				const PSID accountSid,
				const std::wstring& privilege
			)
			{
				if (!accountSid)
					throw Error::Boring32Error("accountSid cannot be null");
				if (privilege.empty())
					throw Error::Boring32Error("Invalid empty privilege");

				// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
				LSAUnicodeString lsaPrivStr(privilege);
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaremoveaccountrights
				const NTSTATUS status = LsaRemoveAccountRights(
					m_handle.get(), // open policy handle
					accountSid,     // target SID
					false,          // do not disable all rights
					lsaPrivStr.Get(),	// privileges
					1               // privilege count
				);
				if (NT_ERROR(status))
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
					const ULONG win32Error = LsaNtStatusToWinError(status);
					throw Error::Win32Error("LsaRemoveAccountRights() failed", win32Error);
				}
			}

		private:
			LSAHandleUniquePtr m_handle;
	};
}

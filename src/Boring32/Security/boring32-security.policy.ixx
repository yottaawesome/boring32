export module boring32:security.policy;
// https://www.mschaef.com/windows_h_is_wierd
//#define WIN32_NO_STATUS
//#include <Windows.h>
//#undef WIN32_NO_STATUS
//#include <ntstatus.h>
//#include <winnt.h>
//#include <Ntsecapi.h>
import std;
import :win32;
import :error;
import :security.lsaunicodestring;
import :raii;

namespace Boring32::Security
{
	using LSAHandleUniquePtr = RAII::IndirectUniquePtr<Win32::LSA_HANDLE, Win32::LsaClose>;
	using LSAHandleSharedPtr = std::shared_ptr<std::remove_pointer_t<Win32::LSA_HANDLE>>;
	LSAHandleSharedPtr CreateLSASharedPtr(Win32::LSA_HANDLE handle)
	{
		return LSAHandleSharedPtr(handle, Win32::LsaClose);
	}

	// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/opening-a-policy-object-handle
	export struct Policy final
	{
		// See https://learn.microsoft.com/en-us/windows/win32/secmgmt/policy-object-access-rights
		Policy(const Win32::ACCESS_MASK desiredAccess)
		{
			Win32::LSA_OBJECT_ATTRIBUTES obj{ 0 };
			Win32::LSA_HANDLE handle;
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaopenpolicy
			// See also https://learn.microsoft.com/en-us/windows/win32/secmgmt/opening-a-policy-object-handle
			Win32::NTSTATUS status = Win32::LsaOpenPolicy(
				nullptr,
				&obj,
				desiredAccess,
				&handle
			);
			if (Win32::NT_ERROR(status))
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
				throw Error::Win32Error(Win32::LsaNtStatusToWinError(status), "LsaOpenPolicy() failed");
			m_handle = LSAHandleUniquePtr(handle);
		}

		void Close()
		{
			m_handle.reset();
		}

		// Policy handle requires POLICY_LOOKUP_NAMES, and optionally POLICY_CREATE_ACCOUNT.
		void AddAccountPrivilege(
			const Win32::PSID accountSid,
			const std::wstring& privilege
		)
		{
			if (not accountSid)
				throw Error::Boring32Error("accountSid cannot be null");
			if (privilege.empty())
				throw Error::Boring32Error("Invalid empty privilege");

			// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
			// See also the return values for LSA: https://learn.microsoft.com/en-us/windows/win32/secmgmt/management-return-values
			LSAUnicodeString lsaPrivStr(privilege);
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaaddaccountrights
			Win32::NTSTATUS status = Win32::LsaAddAccountRights(
				m_handle.get(), // open policy handle
				accountSid,     // target SID
				lsaPrivStr.Get(),		// privileges
				1               // privilege count
			);
			if (Win32::NT_ERROR(status))
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
				throw Error::Win32Error(Win32::LsaNtStatusToWinError(status), "LsaAddAccountRights() failed");
		}

		// Policy handle requires POLICY_LOOKUP_NAMES.
		void RemoveAccountPrivilege(const Win32::PSID accountSid, const std::wstring& privilege)
		{
			if (not accountSid)
				throw Error::Boring32Error("accountSid cannot be null");
			if (privilege.empty())
				throw Error::Boring32Error("Invalid empty privilege");

			// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
			LSAUnicodeString lsaPrivStr(privilege);
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaremoveaccountrights
			Win32::NTSTATUS status = Win32::LsaRemoveAccountRights(
				m_handle.get(), // open policy handle
				accountSid,     // target SID
				false,          // do not disable all rights
				lsaPrivStr.Get(),	// privileges
				1               // privilege count
			);
			if (Win32::NT_ERROR(status))
				// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
				throw Error::Win32Error(Win32::LsaNtStatusToWinError(status), "LsaRemoveAccountRights() failed");
		}

		private:
		LSAHandleUniquePtr m_handle;
	};
}

module;

#include <string>
#include <source_location>
// https://www.mschaef.com/windows_h_is_wierd
//#define WIN32_NO_STATUS
#include <Windows.h>
//#undef WIN32_NO_STATUS
//#include <ntstatus.h>
//#include <winnt.h>
#include <Ntsecapi.h>

module boring32.security:policy;
import boring32.error;

namespace Boring32::Security
{
	// https://blog.katastros.com/a?ID=00750-8d94e2aa-ea28-4faf-b67d-57642f88b0bb
	inline bool NT_SUCCESS(const NTSTATUS status) noexcept
	{ 
		return status >= 0;
	}

	Policy::~Policy() {}
	
	Policy::Policy(const ACCESS_MASK desiredAccess)
	{
		LSA_OBJECT_ATTRIBUTES obj{ 0 };
		LSA_HANDLE handle;
		// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaopenpolicy
		const NTSTATUS status = LsaOpenPolicy(
			nullptr,
			&obj,
			desiredAccess,
			&handle
		);
		if (!NT_SUCCESS(status))
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
			const ULONG win32Error = LsaNtStatusToWinError(status);
			throw Error::Win32Error("LsaOpenPolicy() failed", win32Error);
		}
		m_handle = LSAHandleUniquePtr(handle);
	}

	void Policy::Close()
	{
		m_handle.reset();
	}

	void Policy::AddAccountPrivilege(
		const PSID accountSid,
		const std::wstring& privilege
	)
	{
		if (!accountSid)
			throw Error::Boring32Error("accountSid cannot be null");

		// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
		// https://learn.microsoft.com/en-us/windows/win32/api/lsalookup/ns-lsalookup-lsa_unicode_string
		// See also the return values for LSA: https://learn.microsoft.com/en-us/windows/win32/secmgmt/management-return-values
		LSA_UNICODE_STRING lsaPrivStr{
			.Length = static_cast<unsigned short>(privilege.size() * sizeof(wchar_t)),
			.MaximumLength = static_cast<unsigned short>(privilege.size() * sizeof(wchar_t)),
			.Buffer = const_cast<wchar_t*>(privilege.c_str())
		};
		// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaaddaccountrights
		const NTSTATUS status = LsaAddAccountRights(
			m_handle.get(), // open policy handle
			accountSid,     // target SID
			&lsaPrivStr,	// privileges
			1               // privilege count
		);
		if (!NT_SUCCESS(status))
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
			const ULONG win32Error = LsaNtStatusToWinError(status);
			throw Error::Win32Error("LsaAddAccountRights() failed", win32Error);
		}
	}

	void Policy::RemoveAccountPrivilege(
		const PSID accountSid,
		const std::wstring& privilege
	) 
	{
		if (!accountSid)
			throw Error::Boring32Error("accountSid cannot be null");

		// Based on https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/security/lsapolicy/lsaprivs/LsaPrivs.c
		// https://learn.microsoft.com/en-us/windows/win32/api/lsalookup/ns-lsalookup-lsa_unicode_string
		LSA_UNICODE_STRING lsaPrivStr{
			.Length = static_cast<unsigned short>(privilege.size() * sizeof(wchar_t)),
			.MaximumLength = static_cast<unsigned short>(privilege.size() * sizeof(wchar_t)),
			.Buffer = const_cast<wchar_t*>(privilege.c_str())
		};
		// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsaremoveaccountrights
		const NTSTATUS status = LsaRemoveAccountRights(
			m_handle.get(), // open policy handle
			accountSid,     // target SID
			false,          // do not disable all rights
			&lsaPrivStr,	// privileges
			1               // privilege count
		);
		if (!NT_SUCCESS(status))
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/nf-ntsecapi-lsantstatustowinerror
			const ULONG win32Error = LsaNtStatusToWinError(status);
			throw Error::Win32Error("LsaRemoveAccountRights() failed", win32Error);
		}
	}
}
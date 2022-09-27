module;

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
}
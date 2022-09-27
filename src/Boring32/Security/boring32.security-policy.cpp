module;

#include <source_location>
// https://blog.katastros.com/a?ID=00750-8d94e2aa-ea28-4faf-b67d-57642f88b0bb
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
	inline bool NT_SUCCESS(const NTSTATUS Status) 
	{ 
		return Status >= 0;
	}

	Policy::~Policy() {}
	
	Policy::Policy(ACCESS_MASK desiredAccess)
	{
		LSA_OBJECT_ATTRIBUTES obj{ 0 };
		LSA_HANDLE handle;
		const NTSTATUS status = LsaOpenPolicy(
			nullptr,
			&obj,
			desiredAccess,
			&handle
		);
		if (!NT_SUCCESS(status))
		{
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
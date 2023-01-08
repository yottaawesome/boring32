export module boring32.win32:defs;
import <win32.hpp>;

namespace Boring32::Win32
{
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwmapviewofsection
	using MapViewOfSection = NTSTATUS(*)(
		HANDLE          SectionHandle,
		HANDLE          ProcessHandle,
		PVOID* BaseAddress,
		ULONG_PTR       ZeroBits,
		SIZE_T          CommitSize,
		PLARGE_INTEGER  SectionOffset,
		PSIZE_T         ViewSize,
		unsigned		InheritDisposition,
		ULONG           AllocationType,
		ULONG           Win32Protect
	);
}
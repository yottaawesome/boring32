export module boring32.win32:defs;
import <win32.hpp>;

namespace Boring32::Win32
{
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/managing-memory-sections
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

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwunmapviewofsection
	using UnmapViewOfSection = NTSTATUS (*)(
		HANDLE ProcessHandle,
		PVOID  BaseAddress
	);

	// https://learn.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_object_attributes
	struct OBJECT_ATTRIBUTES 
	{
		ULONG           Length;
		HANDLE          RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG           Attributes;
		PVOID           SecurityDescriptor;
		PVOID           SecurityQualityOfService;
	};

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwcreatesection
	using CreateSection = NTSTATUS (*)(
		PHANDLE            SectionHandle,
		ACCESS_MASK        DesiredAccess,
		OBJECT_ATTRIBUTES* ObjectAttributes,
		PLARGE_INTEGER     MaximumSize,
		ULONG              SectionPageProtection,
		ULONG              AllocationAttributes,
		HANDLE             FileHandle
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwopensection
	using OpenSection = NTSTATUS (*)(
		PHANDLE            SectionHandle,
		ACCESS_MASK        DesiredAccess,
		OBJECT_ATTRIBUTES* ObjectAttributes
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntclose
	using Close = NTSTATUS (*)(
		HANDLE Handle
	);
}
#define NTAPI __stdcall
export module boring32.native:defs;
//import <win32.hpp>;
import boring32.shared;

namespace Boring32::Native
{
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/managing-memory-sections
	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwmapviewofsection
	using MapViewOfSection = Win32::NTSTATUS(*)(
		Win32::HANDLE          SectionHandle,
		Win32::HANDLE          ProcessHandle,
		Win32::PVOID* BaseAddress,
		Win32::ULONG_PTR       ZeroBits,
		Win32::SIZE_T          CommitSize,
		Win32::PLARGE_INTEGER  SectionOffset,
		Win32::PSIZE_T         ViewSize,
		unsigned		InheritDisposition,
		Win32::ULONG           AllocationType,
		Win32::ULONG           Win32Protect
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwunmapviewofsection
	using UnmapViewOfSection = Win32::NTSTATUS (*)(
		HANDLE ProcessHandle,
		PVOID  BaseAddress
	);

	// https://learn.microsoft.com/en-us/windows/win32/api/ntdef/ns-ntdef-_object_attributes
	struct OBJECT_ATTRIBUTES 
	{
		Win32::ULONG           Length;
		Win32::HANDLE          RootDirectory;
		Win32::PUNICODE_STRING ObjectName;
		Win32::ULONG           Attributes;
		Win32::PVOID           SecurityDescriptor;
		Win32::PVOID           SecurityQualityOfService;
	};

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwcreatesection
	using CreateSection = Win32::NTSTATUS (*)(
		Win32::PHANDLE            SectionHandle,
		Win32::ACCESS_MASK        DesiredAccess,
		OBJECT_ATTRIBUTES* ObjectAttributes,
		Win32::PLARGE_INTEGER     MaximumSize,
		Win32::ULONG              SectionPageProtection,
		Win32::ULONG              AllocationAttributes,
		Win32::HANDLE             FileHandle
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwopensection
	using OpenSection = Win32::NTSTATUS (*)(
		Win32::PHANDLE            SectionHandle,
		Win32::ACCESS_MASK        DesiredAccess,
		OBJECT_ATTRIBUTES* ObjectAttributes
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntclose
	using Close = Win32::NTSTATUS (*)(
		Win32::HANDLE Handle
	);
	
	// https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation
	using QuerySystemInformation = Win32::NTSTATUS(NTAPI*)(
		Win32::ULONG SystemInformationClass,
		Win32::PVOID SystemInformation,
		Win32::ULONG SystemInformationLength,
		Win32::PULONG ReturnLength
	);

	// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-zwduplicateobject
	using DuplicateObject = Win32::NTSTATUS(NTAPI*)(
		Win32::HANDLE SourceProcessHandle,
		Win32::HANDLE SourceHandle,
		Win32::HANDLE TargetProcessHandle,
		Win32::PHANDLE TargetHandle,
		Win32::ACCESS_MASK DesiredAccess,
		Win32::ULONG Attributes,
		Win32::ULONG Options
	);

	// See https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntqueryobject
	struct PUBLIC_OBJECT_BASIC_INFORMATION
	{
		Win32::ULONG Attributes;
		Win32::ACCESS_MASK GrantedAccess;
		Win32::ULONG HandleCount;
		Win32::ULONG PointerCount;
		Win32::ULONG Reserved[10];    // reserved for internal use
	};
	struct PUBLIC_OBJECT_TYPE_INFORMATION
	{
		Win32::UNICODE_STRING TypeName;
		Win32::ULONG Reserved[22];    // reserved for internal use
	};
	using QueryObject = Win32::NTSTATUS(NTAPI*)(
		Win32::HANDLE ObjectHandle,
		Win32::ULONG ObjectInformationClass,
		Win32::PVOID ObjectInformation,
		Win32::ULONG ObjectInformationLength,
		Win32::PULONG ReturnLength
	);
}
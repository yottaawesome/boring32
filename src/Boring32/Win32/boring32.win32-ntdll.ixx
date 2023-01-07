export module boring32.win32:ntdll;
import <win32.hpp>;

export namespace Boring32::Win32
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

	class NTDLL final
	{
		public:
			NTDLL();
			NTDLL(const NTDLL&) = delete;
			NTDLL(NTDLL&&) noexcept = delete;

		public:
			NTDLL& operator=(const NTDLL&) = delete;
			NTDLL& operator=(NTDLL&&) noexcept = delete;

		private:
			void Map();

		private:
			MapViewOfSection m_mapViewOfSection;
	};
}

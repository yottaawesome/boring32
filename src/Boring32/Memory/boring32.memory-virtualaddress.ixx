export module boring32.memory:virtualaddress;
import <win32.hpp>;

export namespace Boring32::Memory
{
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
	enum class AllocationType
	{
		Commit = MEM_COMMIT,
		Reserve = MEM_RESERVE,
		Reset = MEM_RESET,
		Undo = MEM_RESET_UNDO
	};

	enum class AllocationTypeOptional
	{
		LargePages = MEM_LARGE_PAGES,
		Physical = MEM_PHYSICAL,
		TopDown = MEM_TOP_DOWN,
		WriteWatch = MEM_WRITE_WATCH
	};

	// https://learn.microsoft.com/en-us/windows/win32/Memory/memory-protection-constants
	enum class MemoryProtection
	{

	};
}
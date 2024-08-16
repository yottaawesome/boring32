export module boring32:memory_functions;
import :computer;

export namespace Boring32::Memory
{
	inline unsigned long GetPageSize() noexcept
	{
		return Computer::GetSystemInfo().dwPageSize;
	}
}
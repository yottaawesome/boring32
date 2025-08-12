export module boring32:memory_functions;
import :computer;

export namespace Boring32::Memory
{
	auto GetPageSize() noexcept -> unsigned long
	{
		return Computer::GetSystemInfo().dwPageSize;
	}
}
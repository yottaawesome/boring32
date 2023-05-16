export module boring32.memory:functions;
import boring32.computer;

export namespace Boring32::Memory
{
	inline unsigned long GetPageSize() noexcept
	{
		return Computer::GetSystemInfo().dwPageSize;
	}
}
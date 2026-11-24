export module boring32:memory.functions;
import :computer;
import :win32;

export namespace Boring32::Memory
{
	auto GetPageSize() noexcept -> Win32::DWORD
	{
		return Computer::GetSystemInfo().dwPageSize;
	}
}
export module Boring32:Memory.Functions;
import :Computer;
import :Win32;

export namespace Boring32::Memory
{
	auto GetPageSize() noexcept -> Win32::DWORD
	{
		return Computer::GetSystemInfo().dwPageSize;
	}
}
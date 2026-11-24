export module Boring32:Sspi.Types;
import std;
import :Win32;
import :Raii;

export namespace Boring32::SSPI
{
	using ContextBufferUniquePtr = RAII::UniquePtr<void, Win32::FreeContextBuffer>;
	using ContextBufferSharedPtr = std::shared_ptr<void>;
	ContextBufferSharedPtr MakeContextBufferSharedPtr(void* ptr)
	{
		return { ptr, Win32::FreeContextBuffer };
	}
}
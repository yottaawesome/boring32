export module boring32:sspi_types;
import std;
import boring32.win32;
import :raii;

export namespace Boring32::SSPI
{
	using ContextBufferUniquePtr = RAII::UniquePtr<void, Win32::FreeContextBuffer>;
	using ContextBufferSharedPtr = std::shared_ptr<void>;
	ContextBufferSharedPtr MakeContextBufferSharedPtr(void* ptr)
	{
		return { ptr, Win32::FreeContextBuffer };
	}
}
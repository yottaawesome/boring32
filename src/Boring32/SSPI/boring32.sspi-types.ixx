export module boring32.sspi:types;
import std;
import std.compat;
import boring32.win32;

export namespace Boring32::SSPI
{
	struct ContextBufferDeleter final
	{
		void operator()(void* ptr)
		{
			Win32::FreeContextBuffer(ptr);
		}
	};
	using ContextBufferUniquePtr = std::unique_ptr<void, ContextBufferDeleter>;
	using ContextBufferSharedPtr = std::shared_ptr<void>;
	ContextBufferSharedPtr MakeContextBufferSharedPtr(void* ptr)
	{
		return { ptr, Win32::FreeContextBuffer };
	}
}
export module boring32:sspi_types;
import boring32.shared;

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
export module boring32.sspi:types;
import std;

import <win32.hpp>;

export namespace Boring32::SSPI
{
	struct ContextBufferDeleter final
	{
		void operator()(void* ptr)
		{
			FreeContextBuffer(ptr);
		}
	};
	using ContextBufferUniquePtr = std::unique_ptr<void, ContextBufferDeleter>;
	using ContextBufferSharedPtr = std::shared_ptr<void>;
	ContextBufferSharedPtr MakeContextBufferSharedPtr(void* ptr)
	{
		return { ptr, FreeContextBuffer };
	}
}
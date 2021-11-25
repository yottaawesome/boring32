module;

#include <memory>
#include <Windows.h>

export module boring32.raii.uniqueptrs;

export namespace Boring32::Raii
{
	struct LocalHeapDeleter final
	{
		void operator()(void* ptr)
		{
			LocalFree(ptr);
		}
	};
	using LocalHeapUniquePtr = std::unique_ptr<void, LocalHeapDeleter>;

	struct DllDeleter final
	{
		void operator()(HMODULE ptr)
		{
			FreeLibrary(ptr);
		}
	};
	using DllUniquePtr = std::unique_ptr<std::remove_pointer<HMODULE>::type, DllDeleter>;
}

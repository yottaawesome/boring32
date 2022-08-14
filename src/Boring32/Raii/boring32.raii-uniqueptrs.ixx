module;

#include <memory>
#include <Windows.h>
#include <sddl.h>

export module boring32.raii:uniqueptrs;

export namespace Boring32::RAII
{
	struct LocalHeapDeleter final
	{
		void operator()(void* ptr)
		{
			LocalFree(ptr);
		}
	};
	template<typename T>
	using LocalHeapUniquePtr = std::unique_ptr<T, LocalHeapDeleter>;

	struct DLLDeleter final
	{
		void operator()(HMODULE ptr)
		{
			FreeLibrary(ptr);
		}
	};
	using DllUniquePtr = std::unique_ptr<std::remove_pointer<HMODULE>::type, DLLDeleter>;

	struct SIDDeleter final
	{
		void operator()(PSID ptr)
		{
			FreeSid(ptr);
		}
	};
	using SIDUniquePtr = std::unique_ptr<std::remove_pointer<PSID>::type, SIDDeleter>;
}

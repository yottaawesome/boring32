export module boring32.raii:uniqueptrs;
import <memory>;
import boring32.win32;

export namespace Boring32::RAII
{
	struct LocalHeapDeleter final
	{
		void operator()(void* ptr)
		{
			Win32::LocalFree(ptr);
		}
	};
	template<typename T>
	using LocalHeapUniquePtr = std::unique_ptr<T, LocalHeapDeleter>;

	struct DLLDeleter final
	{
		void operator()(Win32::HMODULE ptr)
		{
			Win32::FreeLibrary(ptr);
		}
	};
	using DllUniquePtr = std::unique_ptr<std::remove_pointer<Win32::HMODULE>::type, DLLDeleter>;

	struct SIDDeleter final
	{
		void operator()(Win32::PSID ptr)
		{
			Win32::FreeSid(ptr);
		}
	};
	using SIDUniquePtr = std::unique_ptr<std::remove_pointer<Win32::PSID>::type, SIDDeleter>;
}

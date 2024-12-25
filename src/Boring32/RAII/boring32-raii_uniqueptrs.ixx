export module boring32:raii_uniqueptrs;
import std;
import boring32.win32;

export namespace Boring32::RAII
{
	template<auto VDeleter>
	struct Deleter final { void operator()(auto ptr) const noexcept { VDeleter(ptr); } };

	template<typename T, auto VDeleter>
	using UniquePtr = std::unique_ptr<T, Deleter<VDeleter>>;
	template<typename T, auto VDeleter>
	using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleter>>;
	template<typename T>
	using LocalHeapUniquePtr = UniquePtr<T, Win32::LocalFree>;
	using DllUniquePtr = IndirectUniquePtr<Win32::HMODULE, Win32::FreeLibrary>;
	using SIDUniquePtr = IndirectUniquePtr<Win32::PSID, Win32::FreeSid>;
}

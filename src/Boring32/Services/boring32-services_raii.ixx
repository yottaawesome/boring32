export module boring32:services_raii;
import std;
import boring32.win32;
import :raii;

namespace Boring32::Services
{
	using ServiceHandleUniquePtr = RAII::IndirectUniquePtr<Win32::SC_HANDLE, Win32::CloseServiceHandle>;
	using ServiceHandleSharedPtr = std::shared_ptr<std::remove_pointer_t<Win32::SC_HANDLE>>;

	auto CreateSharedPtr(Win32::SC_HANDLE handle) -> ServiceHandleSharedPtr
	{
		return ServiceHandleSharedPtr(handle, Win32::CloseServiceHandle);
	}
}
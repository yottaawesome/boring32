export module boring32.services:raii;
import std;
import std.compat;
import boring32.win32;

namespace Boring32::Services
{
	struct ServiceHandleDeleter final
	{
		void operator()(Win32::SC_HANDLE handle) const noexcept
		{
			Win32::CloseServiceHandle(handle);
		}
	};
	using ServiceHandleUniquePtr = std::unique_ptr<std::remove_pointer<Win32::SC_HANDLE>::type, ServiceHandleDeleter>;
	using ServiceHandleSharedPtr = std::shared_ptr<std::remove_pointer<Win32::SC_HANDLE>::type>;

	ServiceHandleSharedPtr CreateSharedPtr(Win32::SC_HANDLE handle)
	{
		return ServiceHandleSharedPtr(handle, Win32::CloseServiceHandle);
	}
}
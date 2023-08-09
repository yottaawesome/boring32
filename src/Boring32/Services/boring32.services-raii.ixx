export module boring32.services:raii;
import std;

import <win32.hpp>;

namespace Boring32::Services
{
	struct ServiceHandleDeleter final
	{
		void operator()(SC_HANDLE handle) const noexcept
		{
			CloseServiceHandle(handle);
		}
	};
	using ServiceHandleUniquePtr = std::unique_ptr<std::remove_pointer<SC_HANDLE>::type, ServiceHandleDeleter>;
	using ServiceHandleSharedPtr = std::shared_ptr<std::remove_pointer<SC_HANDLE>::type>;

	ServiceHandleSharedPtr CreateSharedPtr(SC_HANDLE handle)
	{
		return ServiceHandleSharedPtr(handle, CloseServiceHandle);
	}
}
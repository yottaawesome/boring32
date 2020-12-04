#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/AsyncFuncs.hpp"

namespace Boring32::Async
{
	void WaitFor(const HANDLE handle)
	{
		WaitFor(handle, INFINITE, false);
	}

	bool WaitFor(const HANDLE handle, const DWORD timeout)
	{
		return WaitFor(handle, timeout, false);
	}

	bool WaitFor(const HANDLE handle, const DWORD timeout, const bool alertable)
	{
		if (handle == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": handle is nullptr");
		
		DWORD status = WaitForSingleObjectEx(handle, timeout, alertable);
		if (status == WAIT_ABANDONED)
			throw std::runtime_error(__FUNCSIG__ ": the wait was abandoned");
		if (status == WAIT_FAILED)
			throw Error::Win32Error(__FUNCSIG__ ": WaitForSingleObjectEx() failed", GetLastError());
		if (status == WAIT_TIMEOUT)
			return false;
		if (status == WAIT_IO_COMPLETION)
			return false;
		return true;
	}

	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll
	)
	{
		// Due to the parameters we pass in, we'll only be getting
		// positive values
		return (DWORD)WaitFor(handles, waitForAll, INFINITE, false);
	}

	int64_t WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const DWORD timeout
	)
	{
		return WaitFor(handles, waitForAll, timeout, false);
	}

	int64_t WaitFor(
		const std::vector<HANDLE>& handles, 
		const bool waitForAll,
		const DWORD timeout,
		const bool alertable
	)
	{
		if (handles.empty())
			throw std::invalid_argument(__FUNCSIG__ ": handles is empty");
		if (handles.size() > MAXIMUM_WAIT_OBJECTS)
		{
			const std::string error = 
				__FUNCSIG__ ": too many handles to wait on: " + std::to_string(handles.size());
			throw std::invalid_argument(error);
		}

		DWORD status = WaitForMultipleObjectsEx(
			(DWORD)handles.size(), 
			&handles[0], 
			waitForAll, 
			timeout, 
			alertable
		);
		if (status == WAIT_ABANDONED)
			throw std::runtime_error(__FUNCSIG__ ": the wait was abandoned");
		if (status == WAIT_FAILED)
			throw Error::Win32Error(__FUNCSIG__ ": WaitForSingleObjectEx() failed", GetLastError());
		if (status == WAIT_TIMEOUT)
			return -1;
		if (status == WAIT_IO_COMPLETION)
			return -1;

		return waitForAll ? 1 : status - WAIT_OBJECT_0;
	}
}
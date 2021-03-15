#include "pch.hpp"
#include <stdexcept>
#include <TlHelp32.h>
#include "include/Raii/Win32Handle.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Async/AsyncFuncs.hpp"
#include "include/Strings/Strings.hpp"

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
		return (DWORD)WaitFor(handles, waitForAll, INFINITE, false);
	}

	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const DWORD timeout
	)
	{
		return WaitFor(handles, waitForAll, timeout, false);
	}

	DWORD WaitFor(
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
		if (status == WAIT_TIMEOUT || status == WAIT_IO_COMPLETION)
			return status;

		return waitForAll ? 0 : status - WAIT_OBJECT_0;
	}

	bool GetProcessIdByName(
		const std::wstring& processName,
		const int sessionIdToMatch,
		DWORD& outResult
	)
	{
		Raii::Win32Handle processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (processesSnapshot == INVALID_HANDLE_VALUE)
			throw Error::Win32Error(
				__FUNCSIG__ ": CreateToolhelp32Snapshot() failed",
				GetLastError()
			);

		PROCESSENTRY32W procEntry{ .dwSize = sizeof(PROCESSENTRY32W) };
		if (Process32FirstW(processesSnapshot.GetHandle(), &procEntry) == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": Process32First() failed",
				GetLastError()
			);

		do
		{
			if (Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
			{
				if (sessionIdToMatch < 0)
				{
					outResult = procEntry.th32ProcessID;
					return true;
				}
				else
				{
					DWORD processSessionId = 0;
					if (ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
					{
						if (processSessionId == sessionIdToMatch)
						{
							outResult = procEntry.th32ProcessID;
							return true;
						}
					}
					else
					{
						throw Error::Win32Error(
							__FUNCSIG__ ": ProcessIdToSessionId() failed",
							GetLastError()
						);
					}
				}
			}
		} while (Process32NextW(processesSnapshot.GetHandle(), &procEntry));

		return false;
	}
}
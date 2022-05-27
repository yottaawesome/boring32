module;

#include <stdexcept>
#include <vector>
#include <source_location>
#include <format>
#include <Windows.h>
#include <TlHelp32.h>

module boring32.async:functions;
import boring32.raii;
import boring32.strings;
import boring32.error;

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
		if (!handle)
			throw Error::Boring32Error(std::source_location::current(), "Handle is nullptr");
		
		switch (const DWORD status = WaitForSingleObjectEx(handle, timeout, alertable))
		{
			case WAIT_OBJECT_0: 
				return true;
			case WAIT_ABANDONED: 
				throw Error::Boring32Error(std::source_location::current(), "The wait was abandoned");
			case WAIT_FAILED: 
				throw Error::Win32Error(std::source_location::current(), "WaitForSingleObjectEx() failed", GetLastError());
			case WAIT_TIMEOUT: 
				return false;
			case WAIT_IO_COMPLETION: 
				return false;
			default: 
				throw Error::Boring32Error(
					std::source_location::current(), 
					std::format("Unknown wait status: {}", status)
				);
		}
	}

	DWORD WaitFor(const std::vector<HANDLE>& handles, const bool waitForAll)
	{
		return WaitFor(handles, waitForAll, INFINITE, false);
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
			throw Error::Boring32Error(std::source_location::current(), "Handle is nullptr");
		if (handles.size() > MAXIMUM_WAIT_OBJECTS)
		{
			throw Error::Boring32Error(
				std::source_location::current(), 
				std::format("Too many handles to wait on: {}", handles.size())
			);
		}

		const DWORD status = WaitForMultipleObjectsEx(
			static_cast<DWORD>(handles.size()), 
			&handles[0], 
			waitForAll, 
			timeout, 
			alertable
		);
		switch (status)
		{
			case WAIT_ABANDONED:
				throw Error::Boring32Error(std::source_location::current(), "The wait was abandoned.");

			case WAIT_TIMEOUT:
			case WAIT_IO_COMPLETION:
				return status;

			case WAIT_FAILED:
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error(
					std::source_location::current(),
					"WaitForSingleObjectEx() failed",
					lastError
				);
			}

			default:
				return waitForAll ? 0 : status - WAIT_OBJECT_0;
		}
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
				std::source_location::current(), 
				"CreateToolhelp32Snapshot() failed",
				GetLastError()
			);

		PROCESSENTRY32W procEntry{ .dwSize = sizeof(PROCESSENTRY32W) };
		if (Process32FirstW(processesSnapshot.GetHandle(), &procEntry) == false)
			throw Error::Win32Error(
				std::source_location::current(), 
				"Process32First() failed",
				GetLastError()
			);

		do
		{
			if (!Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
				continue;

			if (sessionIdToMatch < 0)
			{
				outResult = procEntry.th32ProcessID;
				return true;
			}

			DWORD processSessionId = 0;
			if (!ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
				throw Error::Win32Error(
					std::source_location::current(), 
					"ProcessIdToSessionId() failed",
					GetLastError()
				);

			if (processSessionId == sessionIdToMatch)
			{
				outResult = procEntry.th32ProcessID;
				return true;
			}
		} while (Process32NextW(processesSnapshot.GetHandle(), &procEntry));

		return false;
	}
}
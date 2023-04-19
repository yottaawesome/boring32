module boring32.async:functions;
import boring32.raii;
import boring32.strings;
import boring32.error;
import <stdexcept>;
import <format>;

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
			throw Error::Boring32Error("Handle is nullptr");
		
		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobjectex
		switch (const DWORD status = WaitForSingleObjectEx(handle, timeout, alertable))
		{
			case WAIT_OBJECT_0: 
				return true;
			
			case WAIT_TIMEOUT:
				return false;
			
			case WAIT_IO_COMPLETION:
				return false;
			
			case WAIT_ABANDONED: 
				throw Error::Boring32Error("The wait was abandoned");
			
			case WAIT_FAILED:
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error( 
					"WaitForSingleObjectEx() failed", 
					lastError
				);
			}
			
			default: 
				throw Error::Boring32Error(std::format("Unknown wait status: {}", status));
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
			throw Error::Boring32Error("Handle is nullptr");
		if (handles.size() > MAXIMUM_WAIT_OBJECTS)
		{
			throw Error::Boring32Error(std::format("Too many handles to wait on: {}", handles.size()));
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjectsex
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
				throw Error::Boring32Error("The wait was abandoned.");

			case WAIT_TIMEOUT:
			case WAIT_IO_COMPLETION:
				return status;

			case WAIT_FAILED:
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error(
					"WaitForSingleObjectEx() failed",
					lastError
				);
			}

			default:
				return waitForAll ? 0 : status - WAIT_OBJECT_0;
		}
	}

	std::vector<DWORD> GetProcessIDsByName(
		const std::wstring& processName,
		const int sessionIdToMatch
	)
	{
		if (processName.empty())
			throw Error::Boring32Error("ProcessName cannot be empty.");

		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
		RAII::Win32Handle processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (processesSnapshot == INVALID_HANDLE_VALUE)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"CreateToolhelp32Snapshot() failed",
				lastError
			);
		}

		PROCESSENTRY32W procEntry{ .dwSize = sizeof(PROCESSENTRY32W) };
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32firstw
		if (!Process32FirstW(processesSnapshot.GetHandle(), &procEntry))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error(
				"Process32First() failed",
				lastError
			);
		}

		std::vector<DWORD> results;
		do
		{
			if (!Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
				continue;

			if (sessionIdToMatch < 0)
			{
				results.push_back(procEntry.th32ProcessID);
				continue;
			}

			DWORD processSessionId = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-processidtosessionid
			if (!ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
			{
				const auto lastError = GetLastError();
				throw Error::Win32Error(
					"ProcessIdToSessionId() failed",
					lastError
				);
			}

			if (processSessionId == sessionIdToMatch)
			{
				results.push_back(procEntry.th32ProcessID);
			}
		} while (Process32NextW(processesSnapshot.GetHandle(), &procEntry));
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32nextw

		return results;
	}
}
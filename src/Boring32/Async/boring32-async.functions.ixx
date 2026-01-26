export module boring32:async.functions;
import std;
import :win32;
import :raii;
import :strings;
import :error;
import :concepts;

export namespace Boring32::Async
{
	// https://stackoverflow.com/questions/37918168/pass-stdchronoduration-by-value-or-by-reference-to-const
	// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-signalobjectandwait
	template<Boring32::Concepts::Duration T>
	Win32::WaitResult SignalAndWait(
		Win32::HANDLE const objectToSignal,
		Win32::HANDLE const objectToWaitOn,
		const T timeout,
		const bool alertable
	)
	{
		if (not objectToSignal)
			throw Error::Boring32Error("objectToSignal is nullptr");
		if (not objectToWaitOn)
			throw Error::Boring32Error("objectToWaitOn is nullptr");

		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		const Win32::DWORD result = SignalObjectAndWait(
			objectToSignal, 
			objectToWaitOn, 
			static_cast<Win32::DWORD>(duration_cast<milliseconds>(timeout).count()),
			alertable
		);
		if (result == Win32::WaitResult::Failed)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("SignalObjectAndWait() failed", lastError);
		}
		return WaitResult(result);
	}

	Win32::WaitResult WaitFor(const Win32::HANDLE handle, const Win32::DWORD timeout, const bool alertable)
	{
		if (not handle)
			throw Error::Boring32Error("Handle is nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobjectex
		switch (const Win32::WaitResult status = (Win32::WaitResult)Win32::WaitForSingleObjectEx(handle, timeout, alertable))
		{
			case Win32::WaitResult::Success:
			case Win32::WaitResult::Timeout:
			case Win32::WaitResult::IoCompletion:
				return status;

			case Win32::WaitResult::Abandoned:
				throw Error::Boring32Error("The wait was abandoned");

			case Win32::WaitResult::Failed:
				throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObjectEx() failed");

			default:
				throw Error::Boring32Error(std::format("Unknown wait status: {}", (unsigned long)status));
		}
	}

	inline void WaitFor(const Win32::HANDLE handle)
	{
		WaitFor(handle, Win32::Infinite, false);
	}

	inline Win32::WaitResult WaitFor(const Win32::HANDLE handle, const Win32::DWORD timeout)
	{
		return WaitFor(handle, timeout, false);
	}

	Win32::WaitResult WaitFor(
		const Win32::HANDLE handle,
		const Concepts::Duration auto& time,
		const bool isAlertable
	)
	{
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		return WaitFor(
			handle, 
			static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()),
			isAlertable
		);
	}

	/// <summary>
	///		Waits until one or all of the specified objects are in the signaled state,
	///		an I/O completion routine or asynchronous procedure call (APC) is queued 
	///		to the thread, or the time-out interval elapses.
	/// </summary>
	/// <param name="handles">
	///		The vector of synchronisation handles to wait on. Must not be empty.
	/// </param>
	/// <param name="waitForAll">
	///		Whether to wait for all objects to be signaled.
	/// </param>
	/// <param name="timeout">
	///		The period in milliseconds to wait, or INFINITE.
	/// </param>
	/// <param name="alertable">
	///		Whether the thread enters an alertable wait.
	/// </param>
	/// <returns>
	///		WAIT_TIMEOUT or WAIT_IO_COMPLETION on timeouts or APC executions 
	///		respectively, 0 if waitForAll is true, or the index of the 
	///		handle that was signaled otherwise.
	/// </returns>
	Win32::DWORD WaitFor(
		const std::vector<Win32::HANDLE>& handles,
		const bool waitForAll,
		const Win32::DWORD timeout,
		const bool alertable
	)
		{
		if (handles.empty())
			throw Error::Boring32Error("Handle is nullptr");
		if (handles.size() > Win32::MaximumWaitObjects)
		{
			throw Error::Boring32Error(std::format("Too many handles to wait on: {}", handles.size()));
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjectsex
		const Win32::DWORD status = Win32::WaitForMultipleObjectsEx(
			static_cast<Win32::DWORD>(handles.size()),
			&handles[0], 
			waitForAll, 
			timeout, 
			alertable
		);
		switch (status)
		{
			case Win32::WaitAbandoned:
				throw Error::Boring32Error("The wait was abandoned.");

			case Win32::WaitTimeout:
			case Win32::WaitIoCompletion:
				return status;

			case Win32::WaitFailed:
				throw Error::Win32Error(Win32::GetLastError(), "WaitForSingleObjectEx() failed");

			default:
				return waitForAll ? 0 : status - Win32::WaitObject0;
		}
	}

	inline Win32::DWORD WaitFor(
		const std::vector<Win32::HANDLE>& handles,
		const bool waitForAll
	)
	{
		return WaitFor(handles, waitForAll, Win32::Infinite, false);
	}

	inline Win32::DWORD WaitFor(
		const std::vector<Win32::HANDLE>& handles,
		const bool waitForAll,
		const Win32::DWORD timeout
	)
	{
		return WaitFor(handles, waitForAll, timeout, false);
	}

	template<typename T>
	Win32::DWORD WaitFor(
		const std::vector<Win32::HANDLE>& handles,
		const bool waitForAll,
		const Concepts::Duration auto& time,
		const bool alertable
	)
	{
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		return WaitFor(
			handles, 
			waitForAll, 
			static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()),
			false
		);
	}

	// Why is this here?
	/// <summary>
	///		Find a process IDs by process name.
	/// </summary>
	/// <param name="processName">
	///		The name of the process to search for.
	/// </param>
	/// <param name="sessionIdToMatch">
	///		The session ID of the process to match.
	///		Pass a negative value for this argument
	///		to be ignored.
	/// </param>
	/// <returns>
	///		vector of IDs.
	/// </returns>
	std::vector<Win32::DWORD> GetProcessIDsByName(
		const std::wstring& processName,
		const int sessionIdToMatch
	)
	{
		if (processName.empty())
			throw Error::Boring32Error("ProcessName cannot be empty.");

		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
		RAII::Win32Handle processesSnapshot = Win32::CreateToolhelp32Snapshot(Win32::Th32csSnapProcess, 0);
		if (processesSnapshot == Win32::InvalidHandleValue)
			throw Error::Win32Error(Win32::GetLastError(), "CreateToolhelp32Snapshot() failed");

		Win32::PROCESSENTRY32W procEntry{ .dwSize = sizeof(Win32::PROCESSENTRY32W) };
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32firstw
		if (not Win32::Process32FirstW(processesSnapshot.GetHandle(), &procEntry))
			throw Error::Win32Error(Win32::GetLastError(), "Process32First() failed");

		std::vector<Win32::DWORD> results;
		do
		{
			if (not Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
				continue;

			if (sessionIdToMatch < 0)
			{
				results.push_back(procEntry.th32ProcessID);
				continue;
			}

			Win32::DWORD processSessionId = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-processidtosessionid
			if (not Win32::ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
				throw Error::Win32Error(Win32::GetLastError(), "ProcessIdToSessionId() failed");

			if (processSessionId == sessionIdToMatch)
			{
				results.push_back(procEntry.th32ProcessID);
			}
		} while (Win32::Process32NextW(processesSnapshot.GetHandle(), &procEntry));
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32nextw

		return results;
	}
}

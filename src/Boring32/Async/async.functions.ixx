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
	auto SignalAndWait(
		Win32::HANDLE toSignal,
		Win32::HANDLE toWaitOn,
		T timeout,
		bool alertable
	) -> Win32::WaitResult
	{
		if (not toSignal)
			throw Error::Boring32Error("toSignal is nullptr");
		if (not toWaitOn)
			throw Error::Boring32Error("toWaitOn is nullptr");

		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		auto result = SignalObjectAndWait(
			toSignal,
			toWaitOn,
			static_cast<Win32::DWORD>(duration_cast<milliseconds>(timeout).count()),
			alertable
		);
		if (result == Win32::WaitResult::Failed)
			throw Error::Win32Error{Win32::GetLastError(), "SignalObjectAndWait() failed"};
		return WaitResult(result);
	}

	auto WaitFor(Win32::HANDLE handle, Win32::DWORD timeout, bool alertable) -> Win32::WaitResult
	{
		if (not handle)
			throw Error::Boring32Error("Handle is nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobjectex
		switch (auto status = static_cast<Win32::WaitResult>(Win32::WaitForSingleObjectEx(handle, timeout, alertable)))
		{
			case Win32::WaitResult::Success:
			case Win32::WaitResult::Timeout:
			case Win32::WaitResult::IoCompletion:
				return status;

			case Win32::WaitResult::Abandoned:
				throw Error::Boring32Error("The wait was abandoned");

			case Win32::WaitResult::Failed:
				throw Error::Win32Error{Win32::GetLastError(), "WaitForSingleObjectEx() failed"};

			default:
				throw Error::Boring32Error(std::format("Unknown wait status: {}", (unsigned long)status));
		}
	}

	void WaitFor(Win32::HANDLE handle)
	{
		WaitFor(handle, Win32::Infinite, false);
	}

	auto WaitFor(Win32::HANDLE handle, Win32::DWORD timeout) -> Win32::WaitResult
	{
		return WaitFor(handle, timeout, false);
	}

	auto WaitFor(Win32::HANDLE handle, Concepts::Duration auto&& time, bool isAlertable) 
		-> Win32::WaitResult
	{
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		return WaitFor(
			handle, 
			static_cast<Win32::DWORD>(duration_cast<milliseconds>(time).count()),
			isAlertable
		);
	}

	///	Waits until one or all of the specified objects are in the signaled state,
	///	an I/O completion routine or asynchronous procedure call (APC) is queued 
	///	to the thread, or the time-out interval elapses.
	auto WaitFor(Concepts::RangeOf<Win32::HANDLE> auto&& handles, bool waitForAll, Win32::DWORD timeout, bool alertable) 
		-> Win32::DWORD
	{
		if (handles.empty())
			throw Error::Boring32Error("Handle is nullptr");
		if (handles.size() > Win32::MaximumWaitObjects)
			throw Error::Boring32Error(std::format("Too many handles to wait on: {}", handles.size()));

		// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjectsex
		auto status = Win32::WaitForMultipleObjectsEx(
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
				throw Error::Win32Error{Win32::GetLastError(), "WaitForSingleObjectEx() failed"};

			default:
				return waitForAll ? 0 : status - Win32::WaitObject0;
		}
	}

	auto  WaitFor(
		Concepts::RangeOf<Win32::HANDLE> auto&& handles,
		bool waitForAll
	) -> Win32::DWORD
	{
		return WaitFor(handles, waitForAll, Win32::Infinite, false);
	}

	auto WaitFor(
		Concepts::RangeOf<Win32::HANDLE> auto&& handles,
		bool waitForAll,
		Win32::DWORD timeout
	)
	{
		return WaitFor(handles, waitForAll, timeout, false);
	}

	template<typename T>
	auto WaitFor(
		Concepts::RangeOf<Win32::HANDLE> auto&& handles,
		bool waitForAll,
		Concepts::Duration auto&& time,
		bool alertable
	) -> Win32::DWORD
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
	///	Find a process IDs by process name.
	auto GetProcessIDsByName(
		const std::wstring& processName,
		int sessionIdToMatch
	) -> std::vector<Win32::DWORD>
	{
		if (processName.empty())
			throw Error::Boring32Error("ProcessName cannot be empty.");

		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
		auto processesSnapshot = RAII::Win32Handle{ Win32::CreateToolhelp32Snapshot(Win32::Th32csSnapProcess, 0) };
		if (processesSnapshot == Win32::InvalidHandleValue)
			throw Error::Win32Error{Win32::GetLastError(), "CreateToolhelp32Snapshot() failed"};

		auto procEntry = Win32::PROCESSENTRY32W{ .dwSize = sizeof(Win32::PROCESSENTRY32W) };
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32firstw
		if (not Win32::Process32FirstW(processesSnapshot.GetHandle(), &procEntry))
			throw Error::Win32Error{Win32::GetLastError(), "Process32First() failed"};

		auto results = std::vector<Win32::DWORD>{};
		do
		{
			if (not Strings::DoCaseInsensitiveMatch(procEntry.szExeFile, processName))
				continue;
			if (sessionIdToMatch < 0)
			{
				results.push_back(procEntry.th32ProcessID);
				continue;
			}

			auto processSessionId = Win32::DWORD{};
			// https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-processidtosessionid
			if (not Win32::ProcessIdToSessionId(procEntry.th32ProcessID, &processSessionId))
				throw Error::Win32Error{Win32::GetLastError(), "ProcessIdToSessionId() failed"};
			if (processSessionId == sessionIdToMatch)
				results.push_back(procEntry.th32ProcessID);
		} while (Win32::Process32NextW(processesSnapshot.GetHandle(), &procEntry));
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32nextw

		return results;
	}
}

module;

#include <source_location>;

export module boring32.async:functions;
import <vector>;
import <string>;
import <stdexcept>;
import <format>;
import <chrono>;
import <win32.hpp>;
import boring32.raii;
import boring32.strings;
import boring32.error;
import :concepts;

export namespace Boring32::Async
{
	enum class WaitResult : DWORD
	{
		Success = WAIT_OBJECT_0,
		Timeout = WAIT_TIMEOUT,
		WaitAbandoned = WAIT_ABANDONED, 
		IOComplete = WAIT_IO_COMPLETION
	};

	// https://stackoverflow.com/questions/37918168/pass-stdchronoduration-by-value-or-by-reference-to-const
	// https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-signalobjectandwait
	template<typename T> requires IsDuration<T>
	WaitResult SignalAndWait(
		HANDLE const objectToSignal,
		HANDLE const objectToWaitOn,
		const T timeout,
		const bool alertable
	)
	{
		if (!objectToSignal)
			throw Error::Boring32Error("objectToSignal is nullptr");
		if (!objectToWaitOn)
			throw Error::Boring32Error("objectToWaitOn is nullptr");

		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		const DWORD result = SignalObjectAndWait(
			objectToSignal, 
			objectToWaitOn, 
			static_cast<DWORD>(duration_cast<milliseconds>(timeout).count()),
			alertable
		);
		if (result == WAIT_FAILED)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("SignalObjectAndWait() failed", lastError);
		}
		return WaitResult(result);
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

	inline void WaitFor(const HANDLE handle)
	{
		WaitFor(handle, INFINITE, false);
	}

	inline bool WaitFor(const HANDLE handle, const DWORD timeout)
	{
		return WaitFor(handle, timeout, false);
	}

	template<typename T>
	bool WaitFor(
		const HANDLE handle, 
		const T& time, 
		const bool isAlertable
	) requires IsDuration<T>
	{
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		return WaitFor(
			handle, 
			static_cast<DWORD>(duration_cast<milliseconds>(time).count()),
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

	inline DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll
	)
	{
		return WaitFor(handles, waitForAll, INFINITE, false);
	}

	inline DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const DWORD timeout
	)
	{
		return WaitFor(handles, waitForAll, timeout, false);
	}

	template<typename T>
	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const T& time,
		const bool alertable
	) requires IsDuration<T>
	{
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;
		return WaitFor(
			handles, 
			waitForAll, 
			static_cast<DWORD>(duration_cast<milliseconds>(time).count()),
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

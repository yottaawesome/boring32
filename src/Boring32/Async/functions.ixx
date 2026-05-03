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
	auto SignalAndWait(Win32::HANDLE toSignal, Win32::HANDLE toWaitOn, T timeout, bool alertable) 
		-> Win32::WaitResult
	{
		if (not toSignal)
			throw Error::Boring32Error("toSignal is nullptr");
		if (not toWaitOn)
			throw Error::Boring32Error("toWaitOn is nullptr");

		auto wait = static_cast<Win32::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());
		auto result = SignalObjectAndWait(toSignal, toWaitOn, wait, alertable);
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
				throw Error::Boring32Error(std::format("Unknown wait status: {}", static_cast<unsigned long>(status)));
		}
	}

	auto WaitFor(Win32::HANDLE handle) -> void
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
		auto wait = static_cast<Win32::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
		return WaitFor(handle, wait, isAlertable);
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

	auto WaitFor(Concepts::RangeOf<Win32::HANDLE> auto&& handles, bool waitForAll) -> Win32::DWORD
	{
		return WaitFor(handles, waitForAll, Win32::Infinite, false);
	}

	auto WaitFor(Concepts::RangeOf<Win32::HANDLE> auto&& handles, bool waitForAll, Win32::DWORD timeout) 
		-> Win32::DWORD
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
		auto wait = static_cast<Win32::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
		return WaitFor(handles, waitForAll, wait, false);
	}
}

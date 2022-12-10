export module boring32.async:functions;
import <vector>;
import <string>;
import <win32.hpp>;

export namespace Boring32::Async
{
	void WaitFor(const HANDLE handle);

	bool WaitFor(const HANDLE handle, const DWORD timeout);

	bool WaitFor(const HANDLE handle, const DWORD timeout, const bool alertable);

	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll
	);

	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const DWORD timeout
	);

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
	);

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
	);
}

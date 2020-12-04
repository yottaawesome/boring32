#pragma once
#include <vector>
#include <windows.h>

namespace Boring32::Async
{
	void WaitFor(const HANDLE handle);
	
	bool WaitFor(const HANDLE handle, const DWORD timeout);
	
	bool WaitFor(const HANDLE handle, const DWORD timeout, const bool alertable);
	
	DWORD WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll
	);
	
	int64_t WaitFor(
		const std::vector<HANDLE>& handles,
		const bool waitForAll,
		const DWORD timeout
	);

	int64_t WaitFor(
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
	///		Whether the thread enters an alertable sleep. If an APC routine is 
	///		triggered and interrupts the wait, this function returns -1.
	/// </param>
	/// <returns>
	///		-1 on timeouts or APC interruptions, 1 if waitForAll is true, and the 
	///		index of the handle that was signaled otherwise.
	/// </returns>
	int64_t WaitFor(
		const std::vector<HANDLE>& handles, 
		const bool waitForAll,
		const DWORD timeout,
		const bool alertable
	);
}

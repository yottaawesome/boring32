#pragma once

namespace Boring32::Async
{
	enum class ThreadStatus
	{
		Finished = 0,
		Failure = 1,
		Ready = 2,
		Running = 3,
		Suspended = 4,
		Terminated = 5,
		FinishedWithError = 0,
	};
}

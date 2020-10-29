#pragma once

namespace Boring32::Async
{
	enum class ThreadStatus
	{
		Finished = 0,
		Ready = 1,
		Running = 2,
		Suspended = 3,
		Terminated = 4
	};
}

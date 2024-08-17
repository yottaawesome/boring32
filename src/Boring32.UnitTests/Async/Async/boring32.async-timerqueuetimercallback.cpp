#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	struct Callback
	{
		static void Run(const bool timerOrWaitFired)
		{

		}
	};

	TEST_CLASS(TimerQueueTimerCallback)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			Boring32::Async::TimerQueueTimerCallback<Callback> t;
		}
	};
}
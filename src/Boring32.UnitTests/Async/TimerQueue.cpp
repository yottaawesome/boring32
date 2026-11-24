#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import Boring32;

namespace Async
{
	TEST_CLASS(TimerQueue)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			Boring32::Async::TimerQueue<> t;
			Assert::IsNull(t.GetHandle());
		}
	};
}
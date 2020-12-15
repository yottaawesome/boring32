#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/WaitableTimer.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Async
{
	TEST_CLASS(WaitableTimer)
	{
		TEST_METHOD(TestCreateAnonymousWaitableTimer)
		{
			Boring32::Async::WaitableTimer timer(L"", false, true);
			Assert::IsTrue(timer.GetHandle());
		}
	};
}
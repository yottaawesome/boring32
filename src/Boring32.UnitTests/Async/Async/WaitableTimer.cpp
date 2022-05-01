#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(WaitableTimer)
	{
		TEST_METHOD(TestCreateAnonymousWaitableTimer)
		{
			Boring32::Async::WaitableTimer timer(false, true);
			Assert::IsTrue(timer.GetHandle());
		}

		TEST_METHOD(TestCreateNamedWaitableTimer)
		{
			Boring32::Async::WaitableTimer timer(L"BlahBlah", false, true);
			Assert::IsTrue(timer.GetHandle());
		}
	};
}
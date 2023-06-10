#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(APCThread)
	{
		TEST_METHOD(TestQueueAPC1)
		{
			Boring32::Async::APCThread apcExecutor;
			apcExecutor.Start();
			Assert::IsTrue(apcExecutor.WaitToStart(1000));
			bool test = false;
			apcExecutor.QueueAPC
			(
				[](ULONG_PTR arg)
				{ 
					bool& test = *reinterpret_cast<bool*>(arg);
					test = true;
				},
				reinterpret_cast<ULONG_PTR>(&test)
			);
			apcExecutor.SignalToExit();
			Assert::IsTrue(apcExecutor.Join(2500));
			Assert::IsTrue(test);
		}

		TEST_METHOD(TestQueueAPC2)
		{
			Boring32::Async::APCThread apcExecutor;
			apcExecutor.Start();
			Assert::IsTrue(apcExecutor.WaitToStart(1000));
			bool test = false;
			apcExecutor.QueueAPC(
				[&test]()
				{
					test = true;
				}
			);
			apcExecutor.SignalToExit();
			Assert::IsTrue(apcExecutor.Join(2500));
			Assert::IsTrue(test);
		}

		TEST_METHOD(TestRun)
		{
			Boring32::Async::APCThread apcExecutor;
			apcExecutor.Start();
			Assert::IsTrue(apcExecutor.WaitToStart(1000));
			apcExecutor.SignalToExit();
			Assert::IsTrue(apcExecutor.Join(5000));
			Assert::IsTrue(apcExecutor.GetExitCode() == 0);
		}
	};
}
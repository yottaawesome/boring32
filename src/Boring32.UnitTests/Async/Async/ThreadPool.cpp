#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/ThreadPool.hpp"
#include "Boring32/include/Async/Event.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Async
{
	namespace TestNs = Boring32::Async;

	TEST_CLASS(ThreadPool)
	{
	public:
		TEST_METHOD(TestCreateThreadPool)
		{
			TestNs::ThreadPool pool(1, 10);
		}

		TEST_METHOD(TestCreateThreadPoolInvalidArgs)
		{
			Assert::ExpectException<std::exception>([]() { Boring32::Async::ThreadPool pool(10, 1); });
		}

		TEST_METHOD(TestCreateSubmitWork)
		{
			TestNs::Event event(false, true, false);
			TestNs::ThreadPool pool(1, 10);
			TestNs::ThreadPool::WorkItem<void*> workItem{
				.Callback = [&event](PTP_CALLBACK_INSTANCE, void*, PTP_WORK)
				{
					event.Signal();
				},
				.Parameter = nullptr
			};
			pool.CreateWork(workItem);
			pool.SubmitWork(workItem.Item);
			Assert::IsTrue(event.WaitOnEvent(3000, true));
		}

		TEST_METHOD(TestCreateSubmitWorkMultipleTimes)
		{
			std::atomic<size_t> counter = 0;
			TestNs::ThreadPool pool(1, 10);
			TestNs::ThreadPool::WorkItem<size_t> workItem{
				.Callback = [&counter](PTP_CALLBACK_INSTANCE instance, size_t parameter, PTP_WORK work)
				{
					counter += parameter;
				},
				.Parameter = 1
			};
			pool.CreateWork(workItem);
			
			pool.SubmitWork(workItem.Item);
			pool.SubmitWork(workItem.Item);
			pool.SubmitWork(workItem.Item);

			for (int loop = 0; counter < 3; loop++)
			{
				Sleep(100);
				if (loop > 5)
					throw std::runtime_error("Exceeded wait time");
			}

			Assert::IsTrue(counter == 3);
		}
	};
}

#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/ThreadPool.hpp"
#include "Boring32/include/Async/Event.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Async
{
	TEST_CLASS(ThreadPool)
	{
	public:
		TEST_METHOD(TestCreateThreadPool)
		{
			Boring32::Async::ThreadPool pool(1, 10);
		}

		TEST_METHOD(TestCreateThreadPoolInvalidArgs)
		{
			Assert::ExpectException<std::exception>(
				[]() 
				{
					Boring32::Async::ThreadPool pool(10, 1);
				}
			);
		}

		TEST_METHOD(TestCreateSubmitWork)
		{
			Boring32::Async::Event e(false, true, false);
			Boring32::Async::ThreadPool pool(1, 10);
				

			Boring32::Async::ThreadPool::WorkItem workItem{
				.Callback = [&e](PTP_CALLBACK_INSTANCE instance, void* parameter, PTP_WORK work)
				{
					e.Signal();
				},
				.Parameter = nullptr
			};
			pool.CreateWork(workItem);
			pool.SubmitWork(workItem.Item);
			Assert::IsTrue(e.WaitOnEvent(3000, true));
		}

		TEST_METHOD(TestCreateSubmitWorkMultipleTimes)
		{
			std::atomic<uint32_t> counter = 0;
			Boring32::Async::ThreadPool pool(1, 10);
			Boring32::Async::ThreadPool::WorkItem workItem{
				.Callback = [&counter](PTP_CALLBACK_INSTANCE instance, void* parameter, PTP_WORK work)
				{
					counter++;
				},
				.Parameter = nullptr
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

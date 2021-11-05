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
			std::function<void(PTP_CALLBACK_INSTANCE, void*, PTP_WORK)> lambda = 
				[&e](PTP_CALLBACK_INSTANCE instance, void* parameter, PTP_WORK work)
				{
					e.Signal();
				};
			auto task = pool.CreateWork(lambda, nullptr);
			pool.SubmitWork(task);
			Assert::IsTrue(e.WaitOnEvent(3000, true));
		}
	};
}

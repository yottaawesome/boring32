#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(AsyncFuncs)
	{
		TEST_METHOD(WaitForSingle)
		{
			auto event = Boring32::Async::ManualResetEvent(false, false, L"");
			auto signalThread = std::thread([&event] { event.Signal(); });
			auto guardThread = std::thread(
				[&event]{
					Sleep(200);
					if (not event.WaitOnEvent(0, false))
						Assert::Fail(L"WaitFor1 failed");
				});
			Boring32::Async::WaitFor(event.GetHandle());
			signalThread.join();
			guardThread.join();
		}

		TEST_METHOD(WaitForSingleChrono)
		{
			auto event = Boring32::Async::ManualResetEvent(false, false);
			event.Signal();
			Assert::IsTrue(
				Boring32::Async::WaitFor(
					event.GetHandle(),
					std::chrono::seconds(5),
					false
				) == Boring32::Win32::WaitResult::Success
			);
		}

		TEST_METHOD(WaitForSingleChronoTimeout)
		{
			auto event = Boring32::Async::ManualResetEvent(false, false);
			Assert::IsTrue(
				Boring32::Async::WaitFor(
					event.GetHandle(),
					std::chrono::seconds(1),
					false
				) == Boring32::Win32::WaitResult::Timeout
			);
		}

		TEST_METHOD(WaitForSingleTimeout)
		{
			auto event = Boring32::Async::ManualResetEvent(false, false, L"");
			Assert::IsTrue(Boring32::Async::WaitFor(event.GetHandle(), 100) == Boring32::Win32::WaitResult::Timeout);
		}

		TEST_METHOD(WaitForMultipleOne)
		{
			auto event1 = Boring32::Async::ManualResetEvent(false, false, L"");
			auto event2 = Boring32::Async::ManualResetEvent(false, false, L"");
			std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };
			std::thread signalThread(
				[&event1, &event2]()
				{
					event2.Signal();
				});

			signalThread.join();
			Assert::IsTrue(Boring32::Async::WaitFor(handles, false) == 1);
		}

		TEST_METHOD(WaitForMultipleAll)
		{
			auto event1 = Boring32::Async::ManualResetEvent(false, false, L"");
			auto event2 = Boring32::Async::ManualResetEvent(false, false, L"");
			std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };
			std::thread signalThread(
				[&event1, &event2]()
				{
					event1.Signal();
					event2.Signal();
				});

			signalThread.join();
			Assert::IsTrue(Boring32::Async::WaitFor(handles, false) == 0);
		}

		TEST_METHOD(WaitForMultipleTimeout)
		{
			auto event1 = Boring32::Async::ManualResetEvent(false, false, L"");
			auto event2 = Boring32::Async::ManualResetEvent(false, false, L"");
			std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };

			Assert::IsTrue(Boring32::Async::WaitFor(handles, false, 100) == WAIT_TIMEOUT);
		}

		TEST_METHOD(WaitForSingleApcTest)
		{
			auto status = DWORD{QueueUserAPC([](ULONG_PTR ptr) {}, GetCurrentThread(), 0)};
			if (not status)
				throw std::runtime_error("Failed to QueueUserAPC");

			auto event1 = Boring32::Async::ManualResetEvent(false, false, L"");
			auto event2 = Boring32::Async::ManualResetEvent(false, false, L"");
			auto handles = std::vector<HANDLE>{ event1.GetHandle(), event2.GetHandle() };
			Assert::IsTrue(
				Boring32::Async::WaitFor(handles, false, INFINITE, true) == WAIT_IO_COMPLETION
			);
		}
	};
}
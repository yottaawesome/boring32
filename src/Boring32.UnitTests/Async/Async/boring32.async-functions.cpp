#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(AsyncFuncs)
	{
		public:
			TEST_METHOD(WaitForSingle)
			{
				Boring32::Async::Event event(false, true, false, L"");
				std::thread signalThread([&event]() { event.Signal(); });
				std::thread guardThread([&event]() {
					Sleep(200);
					if (event.WaitOnEvent(0, false) == false)
						Assert::Fail(L"WaitFor1 failed");
				});
				Boring32::Async::WaitFor(event.GetHandle());
				signalThread.join();
				guardThread.join();
			}

			TEST_METHOD(WaitForSingleChrono)
			{
				Boring32::Async::Event event(false, true, false);
				event.Signal();
				Assert::IsTrue(
					Boring32::Async::WaitFor(
						event.GetHandle(),
						std::chrono::seconds(5),
						false
					) == Boring32::Async::WaitResult::Success
				);
			}

			TEST_METHOD(WaitForSingleChronoTimeout)
			{
				Boring32::Async::Event event(false, true, false);
				Assert::IsTrue(
					Boring32::Async::WaitFor(
						event.GetHandle(),
						std::chrono::seconds(1),
						false
					) == Boring32::Async::WaitResult::Timeout
				);
			}

			TEST_METHOD(WaitForSingleTimeout)
			{
				Boring32::Async::Event event(false, true, false, L"");
				Assert::IsTrue(Boring32::Async::WaitFor(event.GetHandle(), 100) == Boring32::Async::WaitResult::Timeout);
			}

			TEST_METHOD(WaitForMultipleOne)
			{
				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
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
				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
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
				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
				std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };

				Assert::IsTrue(Boring32::Async::WaitFor(handles, false, 100) == WAIT_TIMEOUT);
			}

			TEST_METHOD(WaitForSingleApcTest)
			{
				DWORD status = QueueUserAPC(
					[](ULONG_PTR ptr) {},
					GetCurrentThread(),
					0
				);
				if (!status)
					throw std::runtime_error("Failed to QueueUserAPC");

				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
				std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };
				Assert::IsTrue(
					Boring32::Async::WaitFor(handles, false, INFINITE, true) == WAIT_IO_COMPLETION
				);
			}

			TEST_METHOD(TestGetProcessIDByName)
			{
				const auto results = Boring32::Async::GetProcessIDsByName(
					L"explorer.exe", 
					-1
				);
				Assert::IsTrue(!results.empty());
			}
	};
}
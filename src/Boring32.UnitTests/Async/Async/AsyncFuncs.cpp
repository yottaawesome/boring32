#include "pch.h"
#include <thread>
#include "CppUnitTest.h"
#include "Boring32/include/Async/Async.hpp"
#include "Boring32/include/Error/Error.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Async
{
	TEST_CLASS(AsyncFuncs)
	{
		public:
			TEST_METHOD(WaitFor1)
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

			TEST_METHOD(WaitFor2Timeout)
			{
				Boring32::Async::Event event(false, true, false, L"");
				Assert::IsFalse(Boring32::Async::WaitFor(event.GetHandle(), 100));
			}

			TEST_METHOD(WaitFor3WaitOne)
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

			TEST_METHOD(WaitFor3WaitAll)
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

			TEST_METHOD(WaitFor4Timeout)
			{
				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
				std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };

				Assert::IsTrue(Boring32::Async::WaitFor(handles, false, 100) == WAIT_TIMEOUT);
			}

			TEST_METHOD(WaitFor5ApcTest)
			{
				DWORD status = QueueUserAPC(
					[](ULONG_PTR ptr) {},
					GetCurrentThread(),
					0
				);
				if (status == false)
					throw std::runtime_error("Failed to QueueUserAPC");

				Boring32::Async::Event event1(false, true, false, L"");
				Boring32::Async::Event event2(false, true, false, L"");
				std::vector<HANDLE> handles{ event1.GetHandle(), event2.GetHandle() };
				Assert::IsTrue(Boring32::Async::WaitFor(handles, false, INFINITE, true) == WAIT_IO_COMPLETION);
			}
	};
}
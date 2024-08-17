#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(EventLoop)
	{
		public:
			TEST_METHOD(TestEventLoop)
			{
				Boring32::Async::Event event1(false, true, false);
				Boring32::Async::Event event2(false, true, false);

				Boring32::Async::EventLoop eventLoop;
				eventLoop.On(
					event1.GetHandle(),
					[&event2]() {
						event2.Signal();
					}
				);
				event1.Signal();

				Assert::IsTrue(eventLoop.WaitOn(1000, false));
				Assert::IsTrue(event2.WaitOnEvent(0, false));
			}

			TEST_METHOD(TestEventLoop2)
			{
				std::wstring out;
				Boring32::Async::Event event1(false, true, false);
				Boring32::Async::Event event2(false, true, false);

				Boring32::Async::EventLoop eventLoop;
				eventLoop.On(
					event1.GetHandle(),
					[&out]() {
						out += L"1";
					}
				);
				eventLoop.On(
					event2.GetHandle(),
					[&out]() {
						out += L"2";
					}
				);
				event1.Signal();
				event2.Signal();

				Assert::IsTrue(eventLoop.WaitOn(1000, true));
				Assert::IsTrue(out == L"12");
			}

			TEST_METHOD(TestEventErase)
			{
				Boring32::Async::Event event1(false, true, false);
				Boring32::Async::Event event2(false, true, false);
				Boring32::Async::EventLoop eventLoop;
				eventLoop.On(
					event1.GetHandle(),
					[]() {
					}
				);
				eventLoop.On(
					event2.GetHandle(),
					[]() {
					}
				);
				Assert::IsTrue(eventLoop.Size() == 2);
				eventLoop.Erase(event2.GetHandle());
				Assert::IsTrue(eventLoop.Size() == 1);
			}
	};
}
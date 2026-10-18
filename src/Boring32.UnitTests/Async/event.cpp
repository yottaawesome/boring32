#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(Event)
	{
		public:
			TEST_METHOD(TestCreateAnonymousEvent1)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{};
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateAnonymousEvent2)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{};
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateNamedEvent)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{ Boring32::Async::EventCreate{ .Name = L"TestEvent" } };
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestOpenNamedEvent)
			{
				auto testEvent1 = Boring32::Async::ManualResetEvent{ 
					Boring32::Async::EventCreate{ .Name = L"TestEvent" } };
				auto testEvent2 = Boring32::Async::AutoResetEvent{
					Boring32::Async::EventOpen{ .Name = L"TestEvent" } };
				Assert::IsNotNull(testEvent2.GetHandle());
			}

			TEST_METHOD(TestWaitOnEventChrono)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{};

				std::thread([&]() { testEvent.Signal(); }).detach();
				const bool waitSucceeded = testEvent.WaitOnEvent(
					std::chrono::seconds(3), 
					false
				);
				Assert::IsTrue(waitSucceeded);
			}

			TEST_METHOD(TestManualResetEvent)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{};
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				testEvent.Reset();
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestAutoResetEvent)
			{
				auto testEvent = Boring32::Async::AutoResetEvent{};
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}
			TEST_METHOD(TestMoveConstructor)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{ 
					Boring32::Async::EventCreate{
						.IsSignaled = true, 
						.IsInheritable = true, 
						.Name = L"TestEvent"
					}};
				Assert::IsNotNull(testEvent.GetHandle());
				Assert::IsTrue(testEvent.GetName() == L"TestEvent");
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestMoveAssignment)
			{
				auto testEvent = Boring32::Async::ManualResetEvent{ 
					Boring32::Async::EventCreate{
						.IsSignaled = true, 
						.IsInheritable = true, 
						.Name = L"TestEvent"
					}};
				Assert::IsNotNull(testEvent.GetHandle());
				Assert::IsTrue(testEvent.GetName() == L"TestEvent");
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestWaitException)
			{
				Assert::ExpectException<std::exception>(
					[]{
						auto testEvent = Boring32::Async::ManualResetEvent{};
						testEvent.Close();
						Assert::IsTrue(testEvent.WaitOnEvent(0, true));
					});
			}

			TEST_METHOD(TestClose)
			{
				Boring32::Async::ManualResetEvent testEvent{ 
					Boring32::Async::EventCreate{ 
						.IsSignaled = true, 
						.IsInheritable = true, 
						.Name = L"TestEvent" 
					} };
				testEvent.Close();
				Assert::IsNull(testEvent.GetHandle());
			}
	};
}

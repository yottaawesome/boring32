#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	using 
		Boring32::Async::ManualResetEvent,
		Boring32::Async::AutoResetEvent,
		Boring32::Async::EventCreate,
		Boring32::Async::EventOpen
		;

	TEST_CLASS(Event)
	{
		TEST_METHOD(TestCreateAnonymousEvent1)
		{
			auto testEvent = ManualResetEvent{};
			Assert::IsNotNull(testEvent.GetHandle());
		}

		TEST_METHOD(TestCreateAnonymousEvent2)
		{
			auto testEvent = ManualResetEvent{};
			Assert::IsNotNull(testEvent.GetHandle());
		}

		TEST_METHOD(TestCreateNamedEvent)
		{
			auto testEvent = ManualResetEvent{ EventCreate{ .Name = L"TestEvent" } };
			Assert::IsNotNull(testEvent.GetHandle());
		}

		TEST_METHOD(TestOpenNamedEvent)
		{
			auto testEvent1 = ManualResetEvent{ EventCreate{ .Name = L"TestEvent" } };
			auto testEvent2 = AutoResetEvent{ EventOpen{ .Name = L"TestEvent" } };
			Assert::IsNotNull(testEvent2.GetHandle());
		}

		TEST_METHOD(TestWaitOnEventChrono)
		{
			auto testEvent = ManualResetEvent{};

			std::thread{[&] { testEvent.Signal(); }}.detach();
			auto waitSucceeded = testEvent.WaitOnEvent(std::chrono::seconds(3), false);
			Assert::IsTrue(waitSucceeded);
		}

		TEST_METHOD(TestManualResetEvent)
		{
			auto testEvent = ManualResetEvent{};
			testEvent.Signal();
			Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			testEvent.Reset();
			Assert::IsFalse(testEvent.WaitOnEvent(0, true));
		}

		TEST_METHOD(TestAutoResetEvent)
		{
			auto testEvent = AutoResetEvent{};
			testEvent.Signal();
			Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			Assert::IsFalse(testEvent.WaitOnEvent(0, true));
		}
		TEST_METHOD(TestMoveConstructor)
		{
			auto testEvent = ManualResetEvent{ 
				EventCreate{
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
			auto testEvent = ManualResetEvent{ 
				EventCreate{
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
					auto testEvent = ManualResetEvent{};
					testEvent.Close();
					Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				});
		}

		TEST_METHOD(TestClose)
		{
			auto testEvent = ManualResetEvent{
				EventCreate{ 
					.IsSignaled = true, 
					.IsInheritable = true, 
					.Name = L"TestEvent" 
				} };
			testEvent.Close();
			Assert::IsNull(testEvent.GetHandle());
		}
	};
}

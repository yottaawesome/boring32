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
				Boring32::Async::ManualResetEvent testEvent(false, false, L"");
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateAnonymousEvent2)
			{
				Boring32::Async::ManualResetEvent testEvent(false, false);
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateNamedEvent)
			{
				Boring32::Async::ManualResetEvent testEvent(false, false, L"TestEvent");
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestOpenNamedEvent)
			{
				Boring32::Async::ManualResetEvent testEvent1(false, false, L"TestEvent");
				Boring32::Async::AutoResetEvent testEvent2(true, L"TestEvent", SYNCHRONIZE);
				Assert::IsNotNull(testEvent2.GetHandle());
			}

			TEST_METHOD(TestWaitOnEventChrono)
			{
				Boring32::Async::ManualResetEvent testEvent(false, false);

				std::thread([&]() { testEvent.Signal(); }).detach();
				const bool waitSucceeded = testEvent.WaitOnEvent(
					std::chrono::seconds(3), 
					false
				);
				Assert::IsTrue(waitSucceeded);
			}

			TEST_METHOD(TestManualResetEvent)
			{
				Boring32::Async::ManualResetEvent testEvent(false, false);
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				testEvent.Reset();
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestAutoResetEvent)
			{
				Boring32::Async::AutoResetEvent testEvent(false, false);
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Async::ManualResetEvent testEvent1(true, true, L"TestEvent");
				Boring32::Async::ManualResetEvent testEvent2(testEvent1);
				Assert::IsNotNull(testEvent2.GetHandle());
				Assert::IsTrue(testEvent1.GetName() == testEvent1.GetName());
				Assert::IsTrue(testEvent2.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Async::ManualResetEvent testEvent1(true, true, L"TestEvent");
				Boring32::Async::ManualResetEvent testEvent2 = testEvent1;
				Assert::IsNotNull(testEvent2.GetHandle());
				Assert::IsTrue(testEvent1.GetName() == testEvent1.GetName());
				Assert::IsTrue(testEvent2.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Async::ManualResetEvent testEvent(Boring32::Async::ManualResetEvent(true, true, L"TestEvent"));
				Assert::IsNotNull(testEvent.GetHandle());
				Assert::IsTrue(testEvent.GetName() == L"TestEvent");
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Async::ManualResetEvent testEvent = Boring32::Async::ManualResetEvent(true, true, L"TestEvent");
				Assert::IsNotNull(testEvent.GetHandle());
				Assert::IsTrue(testEvent.GetName() == L"TestEvent");
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestWaitException)
			{
				Assert::ExpectException<std::exception>(
					[]{
						Boring32::Async::ManualResetEvent testEvent;
						Assert::IsTrue(testEvent.WaitOnEvent(0, true));
					});
			}

			TEST_METHOD(TestClose)
			{
				Boring32::Async::ManualResetEvent testEvent(true, true, L"TestEvent");
				testEvent.Close();
				Assert::IsNull(testEvent.GetHandle());
			}
	};
}

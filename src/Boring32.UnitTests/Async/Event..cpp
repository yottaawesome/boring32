#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/Event.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Async
{
	TEST_CLASS(Event)
	{
		public:
			TEST_METHOD(TestCreateAnonymousEvent1)
			{
				Boring32::Async::Event testEvent(false, true, false, L"");
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateAnonymousEvent2)
			{
				Boring32::Async::Event testEvent(false, true, false);
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestCreateNamedEvent)
			{
				Boring32::Async::Event testEvent(false, true, false, L"TestEvent");
				Assert::IsNotNull(testEvent.GetHandle());
			}

			TEST_METHOD(TestOpenNamedEvent)
			{
				Boring32::Async::Event testEvent1(false, true, false, L"TestEvent");
				Boring32::Async::Event testEvent2(true, false, L"TestEvent", SYNCHRONIZE);
				Assert::IsNotNull(testEvent2.GetHandle());
			}

			TEST_METHOD(TestManualResetEvent)
			{
				Boring32::Async::Event testEvent(false, true, false, L"");
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				testEvent.Reset();
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestAutoResetEvent)
			{
				Boring32::Async::Event testEvent(false, false, false, L"");
				testEvent.Signal();
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
				Assert::IsFalse(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Async::Event testEvent1(true, true, true, L"TestEvent");
				Boring32::Async::Event testEvent2(testEvent1);
				Assert::IsNotNull(testEvent2.GetHandle());
				Assert::IsTrue(testEvent1.GetName() == testEvent1.GetName());
				Assert::IsTrue(testEvent2.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestAssignment)
			{
				Boring32::Async::Event testEvent1(true, true, true, L"TestEvent");
				Boring32::Async::Event testEvent2 = testEvent1;
				Assert::IsNotNull(testEvent2.GetHandle());
				Assert::IsTrue(testEvent1.GetName() == testEvent1.GetName());
				Assert::IsTrue(testEvent2.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestMove)
			{
				Boring32::Async::Event testEvent = Boring32::Async::Event(true, true, true, L"TestEvent");
				Assert::IsNotNull(testEvent.GetHandle());
				Assert::IsTrue(testEvent.GetName() == L"TestEvent");
				Assert::IsTrue(testEvent.WaitOnEvent(0, true));
			}

			TEST_METHOD(TestWaitException)
			{
				Assert::ExpectException<std::exception>(
					[]{
						Boring32::Async::Event testEvent;
						Assert::IsTrue(testEvent.WaitOnEvent(0, true));
					});
			}
	};
}

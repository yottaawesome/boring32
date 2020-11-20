#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/Event.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Boring32::Async::UnitTests
{
	TEST_CLASS(Event)
	{
		public:
			TEST_METHOD(TestCreateAnonymousEvent)
			{
				Boring32::Async::Event testEvent(false, true, false, L"");
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
	};
}

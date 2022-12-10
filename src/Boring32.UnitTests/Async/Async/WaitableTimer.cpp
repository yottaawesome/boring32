#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(WaitableTimer)
	{
		TEST_METHOD(TestCreateAnonymousWaitableTimer)
		{
			Boring32::Async::WaitableTimer timer(false, true);
			Assert::IsTrue(timer.GetHandle());
		}

		TEST_METHOD(TestCreateNamedWaitableTimer)
		{
			Boring32::Async::WaitableTimer timer(L"BlahBlah", false, true);
			Assert::IsTrue(timer.GetHandle());
		}

		TEST_METHOD(TestClose)
		{
			Boring32::Async::WaitableTimer timer(false, true);
			timer.Close();
			Assert::IsNull(timer.GetHandle());
		}

		TEST_METHOD(TestAnonymousCopyConstructor)
		{
			Boring32::Async::WaitableTimer timer1(false, true);
			Boring32::Async::WaitableTimer timer2(timer1);
			Assert::IsNotNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.IsManualReset());
			Assert::IsTrue(timer2.IsManualReset());
		}

		TEST_METHOD(TestAnonymousCopyAssignment)
		{
			Boring32::Async::WaitableTimer timer1(false, true);
			Boring32::Async::WaitableTimer timer2 = timer1;
			Assert::IsNotNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.IsManualReset());
			Assert::IsTrue(timer2.IsManualReset());
		}

		TEST_METHOD(TestNamedCopyConstructor)
		{
			Boring32::Async::WaitableTimer timer1(L"BlahBlah",false, true);
			Boring32::Async::WaitableTimer timer2(timer1);
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.IsManualReset());
			Assert::IsTrue(timer2.IsManualReset());
			Assert::IsTrue(timer1.GetName() == L"BlahBlah");
			Assert::IsTrue(timer2.GetName() == L"BlahBlah");
		}

		TEST_METHOD(TestNamedCopyAssignment)
		{
			Boring32::Async::WaitableTimer timer1(L"BlahBlah", false, true);
			Boring32::Async::WaitableTimer timer2 = timer1;
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.IsManualReset());
			Assert::IsTrue(timer2.IsManualReset());
			Assert::IsTrue(timer1.GetName() == L"BlahBlah");
			Assert::IsTrue(timer2.GetName() == L"BlahBlah");
		}

		TEST_METHOD(TestAnonymousMoveConstructor)
		{
			Boring32::Async::WaitableTimer timer1(false, true);
			Boring32::Async::WaitableTimer timer2(std::move(timer1));
			Assert::IsNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer2.IsManualReset());
		}

		TEST_METHOD(TestNamedMoveConstructor)
		{
			Boring32::Async::WaitableTimer timer1(L"BlahBlah", false, true);
			Boring32::Async::WaitableTimer timer2(std::move(timer1));
			Assert::IsNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.GetName().empty());
			Assert::IsTrue(timer2.GetName() == L"BlahBlah");
			Assert::IsTrue(timer2.IsManualReset());
		}

		TEST_METHOD(TestAnonymousMoveAssignment)
		{
			Boring32::Async::WaitableTimer timer1(false, true);
			Boring32::Async::WaitableTimer timer2 = std::move(timer1);
			Assert::IsNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer2.IsManualReset());
		}

		TEST_METHOD(TestNamedMoveAssignment)
		{
			Boring32::Async::WaitableTimer timer1(L"BlahBlah", false, true);
			Boring32::Async::WaitableTimer timer2 = std::move(timer1);
			Assert::IsNull(timer1.GetHandle());
			Assert::IsNotNull(timer2.GetHandle());
			Assert::IsTrue(timer1.GetName().empty());
			Assert::IsTrue(timer2.GetName() == L"BlahBlah");
			Assert::IsTrue(timer2.IsManualReset());
		}
	};
}
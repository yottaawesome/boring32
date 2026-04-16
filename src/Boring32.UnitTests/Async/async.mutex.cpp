#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(Mutex)
	{
		void TestMove(const Boring32::Async::Mutex& from, const Boring32::Async::Mutex& to)
		{
			Assert::IsNull(from.GetHandle());
			Assert::IsTrue(to.GetName() == L"Mutex1");
			Assert::IsNotNull(to.GetHandle());
			// Moved non-nullopt string should still be non-nullopt, but should be empty. Handle should be null.
			if (auto fromName = from.GetName(); fromName.has_value())
				Assert::IsTrue(fromName->empty());
		}

		TEST_METHOD(TestCreateAnonymousMutex)
		{
			Boring32::Async::Mutex testMutex(false, false);
			Assert::IsNotNull(testMutex.GetHandle());
		}

		TEST_METHOD(TestCreateNamedMutex)
		{
			Boring32::Async::Mutex testMutex(false, false, L"Mutex1");
			Assert::IsNotNull(testMutex.GetHandle());
			Assert::IsTrue(testMutex.GetName() == L"Mutex1");
		}

		TEST_METHOD(TestOpenNamedMutex)
		{
			Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
			Boring32::Async::Mutex testMutex2(false, false, L"Mutex1", MUTEX_ALL_ACCESS);
			Assert::IsNotNull(testMutex1.GetHandle());
			Assert::IsNotNull(testMutex2.GetHandle());
			Assert::IsTrue(testMutex2.GetName() == L"Mutex1");
		}

		TEST_METHOD(TestMoveConstructor)
		{
			Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
			Boring32::Async::Mutex testMutex2(std::move(testMutex1));
			TestMove(testMutex1, testMutex2);
		}

		TEST_METHOD(TestMoveAssignment)
		{
			Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
			Boring32::Async::Mutex testMutex2 = std::move(testMutex1);
			TestMove(testMutex1, testMutex2);
		}

		TEST_METHOD(TestLock)
		{
			Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
			Assert::IsTrue(testMutex1.Lock(500, false));
		}

		TEST_METHOD(TestLockChrono)
		{
			Boring32::Async::Mutex testMutex1(false, false);
			Assert::IsTrue(testMutex1.Lock(std::chrono::seconds(5), false));
		}

		TEST_METHOD(TestUnlock)
		{
			Boring32::Async::Mutex testMutex(false, false, L"Mutex1");
			Assert::IsTrue(testMutex.Lock(500, false));
			testMutex.Unlock();
		}

		TEST_METHOD(TestGetName)
		{
			Boring32::Async::Mutex testMutex(false, false, L"Mutex1");
			Assert::IsTrue(testMutex.GetName() == L"Mutex1");
		}

		TEST_METHOD(TestLockException)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]()
				{
					Boring32::Async::Mutex testMutex;
					testMutex.Lock(0, true);
				});
		}

		TEST_METHOD(TestLockNoThrow)
		{
			Boring32::Async::Mutex testMutex;
			Assert::IsFalse(testMutex.Lock(0, true, std::nothrow));
		}

		TEST_METHOD(TestTryUnlockWhenOwned)
		{
			Boring32::Async::Mutex testMutex(false, false, L"Mutex1");
			Assert::IsTrue(testMutex.Lock(500, false));
			Assert::IsTrue(testMutex.TryUnlock());
		}

		TEST_METHOD(TestTryUnlockWhenNotOwned)
		{
			Boring32::Async::Mutex testMutex(false, false, L"Mutex1");
			Assert::IsFalse(testMutex.TryUnlock());
		}

		TEST_METHOD(TestTryUnlockOnNullMutex)
		{
			Boring32::Async::Mutex testMutex;
			Assert::IsFalse(testMutex.TryUnlock());
		}

		TEST_METHOD(TestUnlockWhenNotOwnedThrows)
		{
			Assert::ExpectException<Boring32::Error::Win32Error>(
				[]()
				{
					Boring32::Async::Mutex testMutex(false, false);
					testMutex.Unlock();
				});
		}

		TEST_METHOD(TestCreateInvalidNamedMutex)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]()
				{
					Boring32::Async::Mutex testMutex(false, false, L"");
				});
		}
	};
}
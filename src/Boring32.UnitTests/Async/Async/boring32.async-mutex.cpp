#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;
import boring32.error;

namespace Async
{
	TEST_CLASS(Mutex)
	{
		private: 
			void TestCopy(const Boring32::Async::Mutex& testMutex1, const Boring32::Async::Mutex& testMutex2)
			{
				Assert::IsTrue(testMutex1.GetName() == L"Mutex1");
				Assert::IsNotNull(testMutex1.GetHandle());
				Assert::IsTrue(testMutex2.GetName() == L"Mutex1");
				Assert::IsNotNull(testMutex2.GetHandle());
			}

			void TestMove(const Boring32::Async::Mutex& from, const Boring32::Async::Mutex& to)
			{
				Assert::IsTrue(from.GetName().empty());
				Assert::IsNull(from.GetHandle());
				Assert::IsTrue(to.GetName() == L"Mutex1");
				Assert::IsNotNull(to.GetHandle());
			}

		public:
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

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
				Boring32::Async::Mutex testMutex2(testMutex1);
				TestCopy(testMutex1, testMutex2);
			}

			TEST_METHOD(TestCopyAssigment)
			{
				Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
				Boring32::Async::Mutex testMutex2 = testMutex1;
				TestCopy(testMutex1, testMutex2);
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
	};
}
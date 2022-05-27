#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;
import boring32.error;

namespace Async
{
	TEST_CLASS(Mutex)
	{
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

			TEST_METHOD(TestCopyMutex)
			{
				Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
				Boring32::Async::Mutex testMutex2 = testMutex1;
				Assert::IsTrue(testMutex2.GetName() == L"Mutex1");
				Assert::IsNotNull(testMutex2.GetHandle());
				Assert::IsTrue(testMutex2.Lock(0, true));
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Async::Mutex testMutex1(false, false, L"Mutex1");
				Boring32::Async::Mutex testMutex2 = std::move(testMutex1);
				Assert::IsNull(testMutex1.GetHandle());
				Assert::IsTrue(testMutex1.GetName().empty());
				Assert::IsNotNull(testMutex2.GetHandle());
				Assert::IsTrue(testMutex2.GetName() == L"Mutex1");
			}

			TEST_METHOD(TestMutexLockException)
			{
				Assert::ExpectException<Boring32::Error::Boring32Error>(
					[]()
					{
						Boring32::Async::Mutex testMutex;
						testMutex.Lock(0, true);
					});
			}

			TEST_METHOD(TestMutexLockNoThrow)
			{
				Boring32::Async::Mutex testMutex;
				Assert::IsFalse(testMutex.Lock(0, true, std::nothrow));
			}
	};
}
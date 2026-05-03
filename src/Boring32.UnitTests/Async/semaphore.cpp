#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(Semaphore)
	{
	public:
		TEST_METHOD(TestDefaultConstructor)
		{
			Boring32::Async::Semaphore semaphore;
			Assert::IsNull(semaphore.GetHandle());
		}

		TEST_METHOD(TestInvalidConstructor1)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]() { Boring32::Async::Semaphore semaphore(L"", false, 10, 10); });
		}

		TEST_METHOD(TestInvalidConstructor2)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]() { Boring32::Async::Semaphore semaphore(L"", false, 20, 10); });
		}

		TEST_METHOD(TestInvalidConstructor3)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]() { Boring32::Async::Semaphore semaphore(L"", false, 0, 0); });
		}

		TEST_METHOD(TestInvalidConstructor4)
		{
			Assert::ExpectException<Boring32::Error::Win32Error>(
				[]() { Boring32::Async::Semaphore semaphore(L"Blah", false, 10, 10, SEMAPHORE_ALL_ACCESS); });
		}

		TEST_METHOD(TestCopyConstructor)
		{
			Boring32::Async::Semaphore semaphore1(L"HelloMutex", false, 10, 10);
			Boring32::Async::Semaphore semaphore2(semaphore1);
			Assert::IsNotNull(semaphore1.GetHandle());
			Assert::IsNotNull(semaphore2.GetHandle());
			Assert::IsTrue(semaphore1.GetName() == L"HelloMutex");
			Assert::IsTrue(semaphore2.GetName() == L"HelloMutex");
		}

		TEST_METHOD(TestCopyAssignment)
		{
			Boring32::Async::Semaphore semaphore1(L"HelloMutex", false, 10, 10);
			Boring32::Async::Semaphore semaphore2 = semaphore1;
			Assert::IsNotNull(semaphore1.GetHandle());
			Assert::IsNotNull(semaphore2.GetHandle());
			Assert::IsTrue(semaphore1.GetName() == L"HelloMutex");
			Assert::IsTrue(semaphore2.GetName() == L"HelloMutex");
		}

		TEST_METHOD(TestMoveConstructor)
		{
			Boring32::Async::Semaphore semaphore1(L"HelloMutex", false, 10, 10);
			Boring32::Async::Semaphore semaphore2(std::move(semaphore1));
			Assert::IsNull(semaphore1.GetHandle());
			Assert::IsNotNull(semaphore2.GetHandle());
			Assert::IsTrue(semaphore1.GetName().empty());
			Assert::IsTrue(semaphore2.GetName() == L"HelloMutex");
		}

		TEST_METHOD(TestCreateAnonymousSemaphore)
		{
			Boring32::Async::Semaphore semaphore(false, 10, 10);
			Assert::IsNotNull(semaphore.GetHandle());
		}

		TEST_METHOD(TestCreateNamedSemaphore)
		{
			Boring32::Async::Semaphore semaphore(L"HelloMutex", false, 10, 10);
			Assert::IsNotNull(semaphore.GetHandle());
			Assert::IsTrue(semaphore.GetName() == L"HelloMutex");
		}

		TEST_METHOD(TestReleaseSemaphore)
		{
			Boring32::Async::Semaphore semaphore(false, 10, 10);
			semaphore.Release(0);
		}

		TEST_METHOD(TestAcquireChrono)
		{
			Boring32::Async::Semaphore semaphore(false, 10, 10);
			Assert::IsTrue(semaphore.Acquire(std::chrono::seconds(5), false));
		}
	};
}
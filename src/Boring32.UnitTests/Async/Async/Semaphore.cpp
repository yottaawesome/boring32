#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;
import boring32.error;

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

		TEST_METHOD(TestInvalidConstructor)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]() { Boring32::Async::Semaphore semaphore(L"", false, 10, 10); });
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
	};
}
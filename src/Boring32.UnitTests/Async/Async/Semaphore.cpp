#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async.semaphore;
namespace Async
{
	TEST_CLASS(Semaphore)
	{
	public:
		TEST_METHOD(TestCreateAnonymousSemaphore)
		{
			Boring32::Async::Semaphore semaphore(false, 10, 10);
			Assert::IsNotNull(semaphore.GetHandle());
		}

		TEST_METHOD(TestCreateNameSemaphore)
		{
			Boring32::Async::Semaphore semaphore(L"HelloMutex", false, 10, 10);
			Assert::IsNotNull(semaphore.GetHandle());
			Assert::IsTrue(semaphore.GetName() == L"HelloMutex");
		}
	};
}
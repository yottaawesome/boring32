#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/Semaphore.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
	};
}
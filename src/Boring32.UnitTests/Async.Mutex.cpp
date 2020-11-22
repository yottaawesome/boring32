#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Async/Mutex.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Boring32::Async::UnitTests
{
	TEST_CLASS(Mutex)
	{
		public:
			TEST_METHOD(TestCreateMutex)
			{
				Boring32::Async::Mutex testMutex(false, false);
				Assert::IsNotNull(testMutex.GetHandle());
			}
	};
}
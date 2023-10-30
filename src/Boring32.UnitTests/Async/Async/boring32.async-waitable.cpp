#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.async;

namespace Async
{
	TEST_CLASS(Waitable)
	{
		TEST_METHOD(TestThrowingConstructor)
		{
			Assert::ExpectException<std::exception>([]
				{
					Boring32::Async::Waitable<[]{}> w(nullptr);
				});
		}
	};
}
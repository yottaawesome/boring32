#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Async
{
	TEST_CLASS(Waitable)
	{
		TEST_METHOD(TestThrowingConstructor)
		{
			Assert::ExpectException<std::exception>(
				[]{
					Boring32::Async::Waitable<nullptr, []{}> w(nullptr);
					w(std::chrono::seconds(1));
				});
		}
	};
}
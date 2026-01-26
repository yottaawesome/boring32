#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace DataStructures
{
	TEST_CLASS(CappedStack)
	{
		TEST_METHOD(TestInvalidSizeConstructor)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]()
				{
					Boring32::DataStructures::CappedStack<int> stack(0, true);
				});
		}

		TEST_METHOD(TestInvalidPop)
		{
			Assert::ExpectException<Boring32::Error::Boring32Error>(
				[]()
				{
					Boring32::DataStructures::CappedStack<int> stack(5, true);
					stack.Pop();
				});
		}
	};
}
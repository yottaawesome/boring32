#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace DataStructures
{
	TEST_CLASS(CappedStack)
	{
		public:

			TEST_METHOD(TestInvalidSizeConstructor)
			{
				Assert::ExpectException<Boring32::Error::Boring32Error>(
					[]()
					{
						Boring32::DataStructures::CappedStack<int> stack(0, true);
					});
			}

			TEST_METHOD(TestAssignPop)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				for (int i = 0; i < 5; i++)
					stack = i;
				Assert::IsTrue(stack.Pop() == 4);
				Assert::IsTrue(stack.GetSize() == 4);
				for (int i = 0; i < 4; i++)
					Assert::IsTrue(stack[i] == i);
			}

			TEST_METHOD(TestAssignPop2)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				for (int i = 0; i < 5; i++)
					stack = i;
				int i = -1;
				Assert::IsTrue(stack.Pop(i));
				Assert::IsTrue(i == 4);
				Assert::IsTrue(stack.GetSize() == 4);
				for (int i = 0; i < 4; i++)
					Assert::IsTrue(stack[i] == i);
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

			TEST_METHOD(TestAssignEquality)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				for (int i = 0; i < 5; i++)
					stack = i;
				Assert::IsTrue(stack == 4);
			}
	};
}
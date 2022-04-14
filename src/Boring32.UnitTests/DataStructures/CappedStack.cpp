#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.datastructures;

namespace DataStructures
{
	TEST_CLASS(CappedStack)
	{
		public:
			TEST_METHOD(TestSizeConstructor)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				Assert::IsTrue(stack.GetMaxSize() == 5);
				Assert::IsTrue(stack.AddsUniqueOnly());
			}

			TEST_METHOD(TestInvalidSizeConstructor)
			{
				Assert::ExpectException<std::invalid_argument>(
					[]()
					{
						Boring32::DataStructures::CappedStack<int> stack(0, true);
					});
			}

			TEST_METHOD(TestAssignPush)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				for (int i = 0; i < 5; i++)
					stack = i;
				Assert::IsTrue(stack.GetSize() == 5);
				for (int i = 0; i < 5; i++)
					Assert::IsTrue(stack[i] == i);
			}

			TEST_METHOD(TestAssignUniquePush)
			{
				Boring32::DataStructures::CappedStack<int> stack(5, true);
				for (int i = 0; i < 5; i++)
					stack = 1;
				Assert::IsTrue(stack.GetSize() == 1);
				Assert::IsTrue(stack[0] == 1);
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
				Assert::ExpectException<std::runtime_error>(
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
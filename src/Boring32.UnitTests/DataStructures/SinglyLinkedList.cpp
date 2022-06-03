#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.datastructures;

namespace DataStructures
{
	TEST_CLASS(SinglyLinkedList)
	{
		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
			}

			TEST_METHOD(TestAdd)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
			}

			TEST_METHOD(TestDepth)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
				list.Add(8);
				Assert::IsTrue(list.GetDepth() == (USHORT)2);
			}

			TEST_METHOD(EmptyList)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
				list.Add(8);
				list.EmptyList();
				Assert::IsTrue(list.GetDepth() == 0);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list1;
				list1.Add(4);
				list1.Add(8);
				Boring32::DataStructures::SinglyLinkedList<int> list2 = std::move(list1);
				Assert::IsTrue(list1.GetDepth() == 0);
				Assert::IsTrue(list2.GetDepth() == 2);
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list1;
				list1.Add(4);
				list1.Add(8);
				Boring32::DataStructures::SinglyLinkedList<int> list2(std::move(list1));
				Assert::IsTrue(list1.GetDepth() == 0);
				Assert::IsTrue(list2.GetDepth() == 2);
			}

			TEST_METHOD(TestClose)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
				list.Add(8);
				list.Close();
				Assert::IsTrue(list.GetDepth() == 0);
			}

			TEST_METHOD(TestEmptyGetAt)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				Assert::IsNull(list.GetAt(1).get());
			}

			TEST_METHOD(TestPop)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
				list.Add(8);
				list.Add(16);
				Assert::IsTrue(*list.Pop() == 16);
				Assert::IsTrue(*list.Pop() == 8);
				Assert::IsTrue(*list.Pop() == 4);
			}

			TEST_METHOD(TestGet)
			{
				Boring32::DataStructures::SinglyLinkedList<int> list;
				list.Add(4);
				list.Add(8);
				list.Add(16);
				Assert::IsTrue(*list.GetAt(1) == 8);
			}
	};
}
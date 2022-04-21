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
	};
}
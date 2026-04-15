#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace Async
{
	TEST_CLASS(MemoryMappedViewTests)
	{
		using FileMapping = Boring32::Async::FileMapping;
		using MemoryMappedView = Boring32::Async::MemoryMappedView;

		public:
			TEST_METHOD(TestConstructFromFileMapping)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				MemoryMappedView view(fm, Boring32::Win32::FileMapAccess::All);
				Assert::IsNotNull(view.GetPointer());
			}

			TEST_METHOD(TestClose)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				MemoryMappedView view(fm, Boring32::Win32::FileMapAccess::All);
				view.Close();
				Assert::IsNull(view.GetPointer());
			}

			TEST_METHOD(TestMoveConstruct)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				MemoryMappedView view1(fm, Boring32::Win32::FileMapAccess::All);
				void* originalPtr = view1.GetPointer();

				MemoryMappedView view2(std::move(view1));
				Assert::IsNull(view1.GetPointer());
				Assert::AreEqual(originalPtr, view2.GetPointer());
			}

			TEST_METHOD(TestMoveAssign)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				MemoryMappedView view1(fm, Boring32::Win32::FileMapAccess::All);
				void* originalPtr = view1.GetPointer();

				MemoryMappedView view2;
				view2 = std::move(view1);
				Assert::IsNull(view1.GetPointer());
				Assert::AreEqual(originalPtr, view2.GetPointer());
			}

			TEST_METHOD(TestReadWrite)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				MemoryMappedView view(fm, Boring32::Win32::FileMapAccess::All, 0, 0, 256);
				auto* ptr = static_cast<char*>(view.GetPointer());
				ptr[0] = 'H';
				ptr[1] = 'i';
				Assert::AreEqual('H', ptr[0]);
				Assert::AreEqual('i', ptr[1]);
			}

			TEST_METHOD(TestMultipleViewsFromOneMapping)
			{
				FileMapping fm(false, 1024, PAGE_READWRITE);
				MemoryMappedView view1(fm, Boring32::Win32::FileMapAccess::All);
				MemoryMappedView view2(fm, Boring32::Win32::FileMapAccess::All);
				Assert::IsNotNull(view1.GetPointer());
				Assert::IsNotNull(view2.GetPointer());
				// Two views from same mapping should be at different addresses
				Assert::AreNotEqual(view1.GetPointer(), view2.GetPointer());
			}

			TEST_METHOD(TestDefaultConstructedIsNull)
			{
				MemoryMappedView view;
				Assert::IsNull(view.GetPointer());
			}
	};

	TEST_CLASS(TypedMemoryMappedViewTests)
	{
		using FileMapping = Boring32::Async::FileMapping;

		struct TestData
		{
			int x = 0;
			int y = 0;
		};

		public:
			TEST_METHOD(TestTypedViewConstruct)
			{
				FileMapping fm(false, sizeof(TestData), PAGE_READWRITE);
				Boring32::Async::TypedMemoryMappedView<TestData> view(
					fm, Boring32::Win32::FileMapAccess::All
				);
				auto* data = view.GetView();
				Assert::IsNotNull(data);
				Assert::AreEqual(0, data->x);
				Assert::AreEqual(0, data->y);
			}

			TEST_METHOD(TestTypedViewReadWrite)
			{
				FileMapping fm(false, sizeof(TestData), PAGE_READWRITE);
				Boring32::Async::TypedMemoryMappedView<TestData> view(
					fm, Boring32::Win32::FileMapAccess::All
				);
				auto* data = view.GetView();
				data->x = 42;
				data->y = 99;
				Assert::AreEqual(42, data->x);
				Assert::AreEqual(99, data->y);
			}
	};
}
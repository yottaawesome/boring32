#include "pch.h"
#include <thread>
#include <windows.h>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.async;

namespace Async
{
	TEST_CLASS(FileMappingTests)
	{
		using FileMapping = Boring32::Async::FileMapping;

		public:
			TEST_METHOD(TestCreateFileMapping1)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
			}

			TEST_METHOD(TestGetFileSize)
			{
				FileMapping fm(false, 256, PAGE_READWRITE);
				Assert::AreEqual(fm.GetFileSize(), 256ull);
			}

			TEST_METHOD(TestClose)
			{
				FileMapping fm(false, L"DummyName", 256, PAGE_READWRITE);
				fm.Close();
				Assert::IsNull(fm.GetNativeHandle());
				Assert::AreEqual(fm.GetFileSize(), 0ull);
				Assert::IsTrue(fm.GetName().empty());
			}

			TEST_METHOD(TestCopy)
			{
				FileMapping fm1;
				FileMapping fm2(false, L"DummyName", 256, PAGE_READWRITE);
				fm1 = std::move(fm2);

				Assert::IsNotNull(fm1.GetNativeHandle());
				Assert::AreEqual(fm1.GetFileSize(), 256ull);
				Assert::IsFalse(fm1.GetName().empty());
			}

			TEST_METHOD(TestMove)
			{
				FileMapping fm1;
				FileMapping fm2(false, L"DummyName", 256, PAGE_READWRITE);
				fm1 = std::move(fm2);

				Assert::IsNotNull(fm1.GetNativeHandle());
				Assert::AreEqual(fm1.GetFileSize(), 256ull);
				Assert::IsFalse(fm1.GetName().empty());

				Assert::IsNull(fm2.GetNativeHandle());
				Assert::AreEqual(fm2.GetFileSize(), 0ull);
				Assert::IsTrue(fm2.GetName().empty());
			}
	};
}
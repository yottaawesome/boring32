#include "pch.h"
#include <thread>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.async.filemapping;

namespace Async
{
	TEST_CLASS(FileMappingTests)
	{
		using FileMapping = Boring32::Async::FileMapping;

		public:
			TEST_METHOD(TestCreateFileMapping1)
			{
				FileMapping fs(false, 256);
			}

			TEST_METHOD(TestGetFileSize)
			{
				FileMapping fs(false, 256);
				Assert::AreEqual(fs.GetFileSize(), 256ull);
			}

			TEST_METHOD(TestClose)
			{
				FileMapping fs(false, L"DummyName", 256);
				fs.Close();
				Assert::IsNull(fs.GetNativeHandle());
				Assert::AreEqual(fs.GetFileSize(), 0ull);
				Assert::IsTrue(fs.GetName().empty());
			}
	};
}
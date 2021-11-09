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

			TEST_METHOD(TestCGetFileSize)
			{
				FileMapping fs(false, 256);
				Assert::AreEqual(fs.GetFileSize(), 256ull);
			}
	};
}
#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.util;
import boring32.error;

namespace Util
{
	TEST_CLASS(UtilGloballyUniqueID)
	{
		public:
			TEST_METHOD(TestInvalidString)
			{
				Assert::ExpectException<Boring32::Error::Win32Error>(
					[]() { Boring32::Util::GloballyUniqueID id(L"AAAAAAA"); });
			}
	};
}
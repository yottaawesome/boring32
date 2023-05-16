#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(ErrorBase)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Error::ErrorBase<std::exception> error;
				Assert::IsTrue(std::string(error.what()).empty());
			}
	};
}
#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(Win32Error)
	{
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::Win32Error basicError(m_errorMessage);
				Assert::IsTrue(std::string(basicError.what()).contains(m_errorMessage));
			}
	};
}
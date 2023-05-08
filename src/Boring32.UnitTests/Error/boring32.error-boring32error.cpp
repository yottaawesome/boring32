#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(Boring32Error)
	{
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::Boring32Error error(m_errorMessage);
				Assert::IsTrue(std::string(error.what()).contains(m_errorMessage));
			}
	};
}
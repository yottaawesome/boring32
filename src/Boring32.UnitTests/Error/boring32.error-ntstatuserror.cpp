#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(NTStatusError)
	{
		const unsigned m_errorCode = 0xC0000194;
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::NTStatusError basicError(m_errorMessage);
				Assert::IsTrue(std::string(basicError.what()).contains(m_errorMessage));
			}

			TEST_METHOD(TestConstructor2)
			{
				Boring32::Error::NTStatusError basicError(m_errorMessage, m_errorCode);
				Assert::IsTrue(std::string(basicError.what()).contains(m_errorMessage));
				Assert::IsTrue(basicError.GetErrorCode() == m_errorCode);
			}

			TEST_METHOD(TestGetErrorCode)
			{
				Boring32::Error::NTStatusError error(m_errorMessage, m_errorCode);
				Assert::IsTrue(error.GetErrorCode() == m_errorCode);
			}
	};
}
#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

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

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Error::NTStatusError error1(m_errorMessage, m_errorCode);
				Boring32::Error::NTStatusError error2(error1);
				Assert::IsTrue(error1.GetErrorCode() == error2.GetErrorCode());
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Error::NTStatusError error1(m_errorMessage, m_errorCode);
				Boring32::Error::NTStatusError error2 = error1;
				Assert::IsTrue(error1.GetErrorCode() == error2.GetErrorCode());
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Error::NTStatusError error1(m_errorMessage, m_errorCode);
				std::string errorMsg = error1.what();
				Boring32::Error::NTStatusError error2(std::move(error1));
				Assert::IsTrue(m_errorCode == error2.GetErrorCode());
				Assert::IsTrue(errorMsg == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Error::NTStatusError error1(m_errorMessage, m_errorCode);
				std::string errorMsg = error1.what();
				Boring32::Error::NTStatusError error2 = std::move(error1);
				Assert::IsTrue(m_errorCode == error2.GetErrorCode());
				Assert::IsTrue(errorMsg == std::string(error2.what()));
			}
	};
}
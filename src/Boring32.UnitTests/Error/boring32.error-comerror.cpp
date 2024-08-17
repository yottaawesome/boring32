#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

namespace Error
{
	TEST_CLASS(COMError)
	{
		HRESULT m_errorCode = E_ACCESSDENIED;
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::COMError error(m_errorMessage, m_errorCode);
				Assert::IsTrue(error.GetHResult() == m_errorCode);
				Assert::IsTrue(std::string(error.what()).contains(m_errorMessage));
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Error::COMError error1(m_errorMessage, m_errorCode);
				Boring32::Error::COMError error2(error1);
				Assert::IsTrue(error1.GetHResult() == error2.GetHResult());
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Error::COMError error1(m_errorMessage, m_errorCode);
				Boring32::Error::COMError error2 = error1;
				Assert::IsTrue(error1.GetHResult() == error2.GetHResult());
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Error::COMError error1(m_errorMessage, m_errorCode);
				std::string errorMsg = error1.what();
				Boring32::Error::COMError error2(std::move(error1));
				Assert::IsTrue(m_errorCode == error2.GetHResult());
				Assert::IsTrue(errorMsg == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Error::COMError error1(m_errorMessage, m_errorCode);
				std::string errorMsg = error1.what();
				Boring32::Error::COMError error2 = std::move(error1);
				Assert::IsTrue(m_errorCode == error2.GetHResult());
				Assert::IsTrue(errorMsg == std::string(error2.what()));
			}

			TEST_METHOD(TestGetErrorCode)
			{
				Boring32::Error::COMError error(m_errorMessage, m_errorCode);
				Assert::IsTrue(error.GetHResult() == m_errorCode);
			}
	};
}
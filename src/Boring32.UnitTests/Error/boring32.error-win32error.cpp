#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32;

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

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Error::Win32Error error1(0x5, m_errorMessage);
				Boring32::Error::Win32Error error2(error1);
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
				Assert::IsTrue(error1.GetErrorCode() == error2.GetErrorCode());
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Error::Win32Error error1(0x5, m_errorMessage);
				Boring32::Error::Win32Error error2 = error1;
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
				Assert::IsTrue(error1.GetErrorCode() == error2.GetErrorCode());
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Error::Win32Error error1(0x5, m_errorMessage);
				Boring32::Error::Win32Error error2(std::move(error1));
				Assert::IsFalse(std::string(error2.what()).empty());
				Assert::IsTrue(error2.GetErrorCode() == 0x5);
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Error::Win32Error error1(0x5, m_errorMessage);
				Boring32::Error::Win32Error error2 = std::move(error1);
				Assert::IsFalse(std::string(error2.what()).empty());
				Assert::IsTrue(error2.GetErrorCode() == 0x5);
			}

			TEST_METHOD(TestGetErrorCode)
			{
				Boring32::Error::Win32Error error1(0x5, m_errorMessage);
				Boring32::Error::Win32Error error2(0x6, m_errorMessage);
				Assert::IsTrue(error1.GetErrorCode() == 0x5);
				Assert::IsTrue(error2.GetErrorCode() == 0x6);
			}
	};
}
#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import boring32.error;

namespace Error
{
	TEST_CLASS(Boring32Error)
	{
		std::string m_errorMessage = "Some error message";

		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Error::Boring32Error error;
				Assert::IsTrue(!std::string(error.what()).empty());
			}

			TEST_METHOD(TestConstructor)
			{
				Boring32::Error::Boring32Error error(m_errorMessage);
				Assert::IsTrue(std::string(error.what()).contains(m_errorMessage));
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Error::Boring32Error error1(m_errorMessage);
				Boring32::Error::Boring32Error error2(error1);
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Error::Boring32Error error1(m_errorMessage);
				Boring32::Error::Boring32Error error2 = error1;
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Error::Boring32Error error1(m_errorMessage);
				Boring32::Error::Boring32Error error2(std::move(error1));
				Assert::IsTrue(std::string(error1.what()).empty());
				Assert::IsTrue(std::string(error2.what()).contains(m_errorMessage));
			}
	};
}
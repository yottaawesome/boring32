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

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Error::ErrorBase<std::exception> error1("Oh no");
				Boring32::Error::ErrorBase<std::exception> error2(error1);
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Error::ErrorBase<std::exception> error1("Oh no");
				Boring32::Error::ErrorBase<std::exception> error2 = error1;
				Assert::IsTrue(std::string(error1.what()) == std::string(error2.what()));
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Error::ErrorBase<std::exception> error1("Oh no");
				Boring32::Error::ErrorBase<std::exception> error2(std::move(error1));
				Assert::IsTrue(std::string(error1.what()).empty());
				Assert::IsFalse(std::string(error2.what()).empty());
				Assert::IsTrue(error1.GetStacktrace().empty());
				Assert::IsFalse(error2.GetStacktrace().empty());
			}

			TEST_METHOD(TestMoveAssignment)
			{
				Boring32::Error::ErrorBase<std::exception> error1("Oh no");
				Boring32::Error::ErrorBase<std::exception> error2 = std::move(error1);
				Assert::IsTrue(std::string(error1.what()).empty());
				Assert::IsFalse(std::string(error2.what()).empty());
				Assert::IsTrue(error1.GetStacktrace().empty());
				Assert::IsFalse(error2.GetStacktrace().empty());
			}
	};
}
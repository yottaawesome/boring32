#include "pch.h"

import boring32;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Strings
{
	TEST_CLASS(Strings)
	{
		public:
			TEST_METHOD(TestTokeniseString1)
			{
				std::wstring test = L"Hello,World,";
				std::vector strings = Boring32::Strings::TokeniseString(test, L",");
				Assert::IsTrue(strings.size() == 3);
			}

			TEST_METHOD(TestTokeniseString2)
			{
				std::wstring test = L"";
				std::vector<std::wstring> strings = Boring32::Strings::TokeniseString(test, L",");
				Assert::IsTrue(strings.size() == 1);
			}
			
			TEST_METHOD(TestConvertWString)
			{
				const std::wstring test = L"blah";
				const std::string converted = Boring32::Strings::ConvertString(test);
				Assert::IsTrue(converted == "blah");
			}

			TEST_METHOD(TestConvertString)
			{
				const std::string test = "blah";
				const std::wstring converted = Boring32::Strings::ConvertString(test);
				Assert::IsTrue(converted == L"blah");
			}

			TEST_METHOD(TestToLower)
			{
				const std::wstring test = L"BLAH";
				const std::wstring converted = Boring32::Strings::ToLower(test);
				Assert::IsTrue(converted == L"blah");
			}

			TEST_METHOD(TestToUpper)
			{
				const std::wstring test = L"blah";
				const std::wstring converted = Boring32::Strings::ToUpper(test);
				Assert::IsTrue(converted == L"BLAH");
			}
	};
}
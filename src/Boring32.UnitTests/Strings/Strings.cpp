#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Strings/Strings.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Strings
{
	TEST_CLASS(Strings)
	{
		public:
			TEST_METHOD(TestTokeniseString1)
			{
				std::wstring test = L"Hello,World,";
				std::vector<std::wstring> strings = Boring32::Strings::TokeniseString(test, L",");
				Assert::IsTrue(strings.size() == 3);
			}

			TEST_METHOD(TestTokeniseString2)
			{
				std::wstring test = L"";
				std::vector<std::wstring> strings = Boring32::Strings::TokeniseString(test, L",");
				Assert::IsTrue(strings.size() == 1);
			}
	};
}
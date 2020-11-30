#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Strings/Strings.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Boring32::Strings::UnitTests
{
	TEST_CLASS(Strings)
	{
		public:
			TEST_METHOD(TestTokeniseString)
			{
				std::wstring test = L"Hello,World,Haha";
				std::vector<std::wstring> strings = Boring32::Strings::TokeniseString(test, L",");
				Assert::IsTrue(strings.size() == 3);
			}
	};
}
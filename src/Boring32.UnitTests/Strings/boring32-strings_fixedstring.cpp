#include "pch.h"

import boring32;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Strings
{
	TEST_CLASS(FixedString)
	{
	public:
		TEST_METHOD(TestSize)
		{
			Boring32::Strings::FixedStringA str = "AAA";
			Assert::IsTrue(str.Size() == 3);
		}

		TEST_METHOD(TestView)
		{
			Boring32::Strings::FixedStringA str = "AAA";
			std::string_view v = str.ToView();
			Assert::IsTrue(v.size() == 3);
		}

		TEST_METHOD(TestString)
		{
			Boring32::Strings::FixedStringA str = "AAA";
			std::string v = str.ToString();
			Assert::IsTrue(v.size() == 3);
		}

		TEST_METHOD(TestAppend)
		{
			Boring32::Strings::FixedStringA strA = "A";
			Boring32::Strings::FixedStringA strB = "B";
			auto strC = strA + strB;
			Assert::IsTrue(strC.Size() == 2);
			Assert::IsTrue(strC == "AB");
		}

		TEST_METHOD(TestEqualityA)
		{
			Boring32::Strings::FixedStringA strA = "A";
			Assert::IsTrue(strA == "A");
		}

		TEST_METHOD(TestEqualityB)
		{
			Boring32::Strings::FixedStringA strA = "A";
			Assert::IsFalse(strA == "B");
		}

		TEST_METHOD(TestEqualityC)
		{
			Boring32::Strings::FixedStringA strA = "A";
			Assert::IsFalse(strA == "AB");
		}
	};
}
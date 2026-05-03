#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Clipboard
{
	const auto testString = std::wstring{ L"aaa" };
	TEST_CLASS(Operation)
	{
		TEST_METHOD(TestSet)
		{
			Boring32::Clipboard::Operation { nullptr }
				.SetData(testString);
		}

		TEST_METHOD(TestGet)
		{
			auto expected = std::wstring{ L"aaa" };
			auto actual = 
				Boring32::Clipboard::Operation{ nullptr }
					.SetData(expected)
					.GetUnicodeText();
			Assert::AreEqual(expected, actual);
		}
	};
}

#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Security
{
	TEST_CLASS(SecurityIdentifier)
	{
		TEST_METHOD(TestDefaultConstructor)
		{
			Boring32::Security::SecurityIdentifier s;
			Assert::IsFalse(s.HasValue());
		}

		TEST_METHOD(TestCreate)
		{
			Boring32::Security::SecurityIdentifier s(L"S-1-5-18");
			Assert::IsTrue(s.HasValue());
		}

		TEST_METHOD(TestToSidString)
		{
			Boring32::Security::SecurityIdentifier s(L"S-1-5-18");
			Assert::IsTrue(s.ToSidString() == L"S-1-5-18");
		}
	};
}
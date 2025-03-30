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

		TEST_METHOD(TestCopyConstructor)
		{
			Boring32::Security::SecurityIdentifier s1(L"S-1-5-18");
			Boring32::Security::SecurityIdentifier s2(s1);
			Assert::IsTrue(s1.ToSidString() == L"S-1-5-18");
			Assert::IsTrue(s2.ToSidString() == L"S-1-5-18");
		}

		TEST_METHOD(TestCopyAssignment)
		{
			Boring32::Security::SecurityIdentifier s1(L"S-1-5-18");
			Boring32::Security::SecurityIdentifier s2 = s1;
			Assert::IsTrue(s1.ToSidString() == L"S-1-5-18");
			Assert::IsTrue(s2.ToSidString() == L"S-1-5-18");
		}

		TEST_METHOD(TestMoveConstructor)
		{
			Boring32::Security::SecurityIdentifier s1(L"S-1-5-18");
			Boring32::Security::SecurityIdentifier s2(std::move(s1));
			Assert::IsFalse(s1.HasValue());
			Assert::IsTrue(s2.ToSidString() == L"S-1-5-18");
		}

		TEST_METHOD(TestMoveAssignment)
		{
			Boring32::Security::SecurityIdentifier s1(L"S-1-5-18");
			Boring32::Security::SecurityIdentifier s2 = std::move(s1);
			Assert::IsFalse(s1.HasValue());
			Assert::IsTrue(s2.ToSidString() == L"S-1-5-18");
		}

		TEST_METHOD(TestLookupName)
		{
			Boring32::Security::SecurityIdentifier s1(L"S-1-5-18");
			s1.LookupName();
		}
	};
}
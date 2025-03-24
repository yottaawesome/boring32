#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;
import boring32.win32;

namespace Security
{
    TEST_CLASS(Token)
    {
        TEST_METHOD(TestConstructor)
        {
            Boring32::Security::Token t(Boring32::Win32::TokenAllAccess);
            Assert::IsTrue(t);
        }

        TEST_METHOD(TestMoveConstructor)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2(std::move(t1));
            Assert::IsFalse(t1);
            Assert::IsTrue(t2);
        }

        TEST_METHOD(TestMoveAssignment)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2 = std::move(t1);
            Assert::IsFalse(t1);
            Assert::IsTrue(t2);
        }

        TEST_METHOD(TestCopyConstructor)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2(t1);
            Assert::IsTrue(t1);
            Assert::IsTrue(t2);
        }

        TEST_METHOD(TestCopyAssignment)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2 = t1;
            Assert::IsTrue(t1);
            Assert::IsTrue(t2);
        }

        TEST_METHOD(TestIsPrimary)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Assert::IsTrue(t1.IsPrimary());
        }

        TEST_METHOD(TestToImpersonation)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2 = t1.ToImpersonationToken();
            Assert::IsTrue(t2.IsImpersonation());
        }
    };
}
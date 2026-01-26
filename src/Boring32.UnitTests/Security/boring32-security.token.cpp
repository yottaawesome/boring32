#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Security
{
    TEST_CLASS(Token)
    {
        TEST_METHOD(TestConstructor)
        {
            Boring32::Security::Token t(Boring32::Win32::TokenAllAccess);
            Assert::IsTrue(t);
        }

        TEST_METHOD(TestDuplicateConstructor)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Boring32::Security::Token t2(t1.GetToken(), false);
            Assert::IsTrue(t1);
            Assert::IsTrue(t2);
            Assert::IsTrue(t1.IsPrimary());
            Assert::IsTrue(t2.IsPrimary());
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

        TEST_METHOD(TestGetStatistics)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            t1.GetStatistics();
        }

        TEST_METHOD(TestGetGroups)
        {
            Boring32::Security::Token t1(Boring32::Win32::TokenAllAccess);
            Assert::IsFalse(t1.GetGroups().empty());
        }

        TEST_METHOD(TestCheckMembership)
        {
            // SECURITY_AUTHENTICATED_USER_RID
            Boring32::Security::SecurityIdentifier sid(L"S-1-5-11");
            Boring32::Security::Token token(Boring32::Win32::TokenAllAccess);
            Assert::IsTrue(token.Boring32::Win32::TokenAllAccess(sid.GetSid()));
        }

        TEST_METHOD(TestGetUser)
        {
            Boring32::Security::Token token(Boring32::Win32::TokenAllAccess);
            Boring32::Security::SidAndAttributes user = token.GetUser();
        }
    };
}
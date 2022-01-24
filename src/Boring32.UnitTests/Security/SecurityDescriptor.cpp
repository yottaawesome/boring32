#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.security.securitydescriptor;

namespace Security
{
	TEST_CLASS(SecurityDescriptor)
	{
        const std::wstring DescriptorString = std::format(
            L"{}:{}{}{}{}",
            L"D",                       // Discretionary ACL
            L"(D;OICI;GA;;;BG)",        // Deny access to built-in guests
            L"(D;OICI;GA;;;AN)",        // Deny access to anonymous logon
            L"(A;OICI;GRGWGX;;;AU)",    // Allow read/write/execute to authenticated 
            L"(A;OICI;GA;;;BA)"         // Allow full control to administrators
        );

		public:
			TEST_METHOD(TestConstructor)
			{
                Boring32::Security::SecurityDescriptor sd(DescriptorString);
			}

            TEST_METHOD(TestGetDescriptor)
            {
                Boring32::Security::SecurityDescriptor sd(DescriptorString);
                Assert::IsNotNull(sd.GetDescriptor());
            }

            TEST_METHOD(TestGetDescriptorString)
            {
                Boring32::Security::SecurityDescriptor sd(DescriptorString);
                Assert::IsTrue(sd.GetDescriptorString() == DescriptorString);
            }

            TEST_METHOD(TestAnonymousMoveAssignment)
            {
                Boring32::Security::SecurityDescriptor sd1 = Boring32::Security::SecurityDescriptor(DescriptorString);
                Assert::IsNotNull(sd1.GetDescriptor());
                Assert::IsTrue(sd1.GetDescriptorString() == DescriptorString);
            }

            TEST_METHOD(TestMoveAssignment)
            {
                Boring32::Security::SecurityDescriptor sd1(DescriptorString);
                Boring32::Security::SecurityDescriptor sd2 = std::move(sd1);
                Assert::IsNull(sd1.GetDescriptor());
                Assert::IsTrue(sd1.GetDescriptorString().empty());
                Assert::IsNotNull(sd2.GetDescriptor());
                Assert::IsTrue(sd2.GetDescriptorString() == DescriptorString);
            }
    };
}
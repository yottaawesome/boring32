#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.crypto;

namespace Crypto
{
	TEST_CLASS(SecureString)
	{
		public:
			TEST_METHOD(TestSecureStringEncryptDecrypt)
			{
				Boring32::Crypto::SecureString secureString;
				secureString.SetValueAndEncrypt(L"TEST VALUE");

				std::wstring out;
				secureString.DecryptCopyAndReencrypt(out);
				Assert::IsTrue(out == L"TEST VALUE");
			}

			TEST_METHOD(TestSecureStringCast)
			{
				Boring32::Crypto::SecureString secureString;
				secureString.SetValueAndEncrypt(L"TEST VALUE");

				std::wstring result = (std::wstring)secureString;
				Assert::IsTrue(result == L"TEST VALUE");
			}

			TEST_METHOD(TestEquals)
			{
				Boring32::Crypto::SecureString secureString;
				secureString.SetValueAndEncrypt(L"TEST VALUE");
				Assert::IsTrue(secureString == L"TEST VALUE");
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Crypto::SecureString secureString1;
				secureString1.SetValueAndEncrypt(L"TEST VALUE");
				Boring32::Crypto::SecureString secureString2(secureString1);
				Assert::IsTrue(secureString1 == secureString2);
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Crypto::SecureString secureString1;
				secureString1.SetValueAndEncrypt(L"TEST VALUE");
				Boring32::Crypto::SecureString secureString2 = secureString1;
				Assert::IsTrue(secureString1 == secureString2);
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Crypto::SecureString secureString1;
				secureString1.SetValueAndEncrypt(L"TEST VALUE");
				Boring32::Crypto::SecureString secureString2(std::move(secureString1));
				Assert::IsFalse(secureString1.HasData());
				Assert::IsTrue(secureString2 == L"TEST VALUE");
			}
	};
}
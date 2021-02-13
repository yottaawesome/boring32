#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Crypto/SecureString.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
				secureString.DecryptAndCopy(out);
				Assert::IsTrue(out == L"TEST VALUE");
			}

			TEST_METHOD(TestSecureStringCast)
			{
				Boring32::Crypto::SecureString secureString;
				secureString.SetValueAndEncrypt(L"TEST VALUE");

				std::wstring result = (std::wstring)secureString;
				Assert::IsTrue(result == L"TEST VALUE");
			}
	};
}
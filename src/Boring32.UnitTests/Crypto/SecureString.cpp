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
				secureString.SetValue(L"TEST VALUE");

				std::wstring out;
				secureString.GetValue(out);
				Assert::IsTrue(out == L"TEST VALUE");
			}
	};
}
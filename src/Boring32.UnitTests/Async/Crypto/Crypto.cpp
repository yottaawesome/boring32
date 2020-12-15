#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Crypto/CryptoFuncs.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Crypto
{
	TEST_CLASS(CryptoFuncs)
	{
		public:
			TEST_METHOD(TestEncryptString)
			{
				std::vector<std::byte> encryptedData =
					Boring32::Crypto::EncryptString(
						L"Basic description",
						L"Hello, world!"
					);
				Assert::IsFalse(encryptedData.empty());
			}

			TEST_METHOD(TestDecryptString)
			{
				std::wstring description = L"Basic description";
				std::wstring data = L"Hello, world";

				std::vector<std::byte> encryptedData =
					Boring32::Crypto::EncryptString(
						description,
						data
					);
				Assert::IsFalse(encryptedData.empty());

				std::wstring descriptionOut;
				std::wstring decryptedString = 
					Boring32::Crypto::DecryptString(encryptedData, descriptionOut);

				Assert::IsTrue(descriptionOut == description);
				Assert::IsTrue(decryptedString == data);
			}
	};
}
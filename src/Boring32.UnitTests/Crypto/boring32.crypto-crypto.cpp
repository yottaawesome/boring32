#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32;

namespace Crypto
{
	TEST_CLASS(CryptoFuncs)
	{
		public:
			TEST_METHOD(TestEncryptString)
			{
				std::vector encryptedData =
					Boring32::Crypto::Encrypt(
						L"Hello, world!",
						L"TestPassword",
						L"Basic description"
					);
				Assert::IsFalse(encryptedData.empty());
			}

			TEST_METHOD(TestDecryptString)
			{
				std::wstring description = L"Basic description";
				std::wstring password = L"TestPassword";
				std::wstring data = L"Hello, world";

				std::vector<std::byte> encryptedData =
					Boring32::Crypto::Encrypt(
						data,
						password,
						description
					);
				Assert::IsFalse(encryptedData.empty());

				std::wstring descriptionOut;
				std::wstring decryptedString = 
					Boring32::Crypto::DecryptString(
						encryptedData,
						password,
						descriptionOut
					);

				Assert::IsTrue(descriptionOut == description);
				Assert::IsTrue(decryptedString == data);
			}
	};
}
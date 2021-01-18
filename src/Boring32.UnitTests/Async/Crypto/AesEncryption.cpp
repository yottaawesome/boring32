#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Crypto/AesEncryption.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Crypto
{
	TEST_CLASS(AesEncryption)
	{
		public:
			TEST_METHOD(TestCreateAesEncryption)
			{
				Boring32::Crypto::AesEncryption aes;
				Assert::IsTrue(aes.GetHandle() != nullptr);
			}

			TEST_METHOD(TestGetObjectByteSize)
			{
				Boring32::Crypto::AesEncryption aes;
				Assert::IsTrue(aes.GetObjectByteSize() > 0);
			}
			
			TEST_METHOD(TestSetChainingMode)
			{
				Boring32::Crypto::AesEncryption aes;
				aes.SetChainingMode(Boring32::Crypto::ChainingMode::CipherBlockChaining);
				aes.SetChainingMode(Boring32::Crypto::ChainingMode::CbcMac);
				aes.SetChainingMode(Boring32::Crypto::ChainingMode::CipherFeedback);
				aes.SetChainingMode(Boring32::Crypto::ChainingMode::ElectronicCodebook);
				aes.SetChainingMode(Boring32::Crypto::ChainingMode::GaloisCounterMode);
			}

			TEST_METHOD(TestGenerateSymmetricKey)
			{
				Boring32::Crypto::AesEncryption aes;
				static std::vector<std::byte> rgbAES128Key
				{
					std::byte{0x00},
					std::byte{0x01},
					std::byte{0x02},
					std::byte{0x03},
					std::byte{0x04},
					std::byte{0x05},
					std::byte{0x06},
					std::byte{0x07},
					std::byte{0x08},
					std::byte{0x09},
					std::byte{0x0A},
					std::byte{0x0B},
					std::byte{0x0C},
					std::byte{0x0D},
					std::byte{0x0E},
					std::byte{0x0F}
				};
				Assert::IsNotNull(aes.GenerateSymmetricKey(rgbAES128Key).GetHandle());
			}

			TEST_METHOD(TestEncryptDecrypt)
			{
				Boring32::Crypto::AesEncryption aes;
				static std::vector<std::byte> rgbAES128Key
				{
					std::byte{0x00},
					std::byte{0x01},
					std::byte{0x02},
					std::byte{0x03},
					std::byte{0x04},
					std::byte{0x05},
					std::byte{0x06},
					std::byte{0x07},
					std::byte{0x08},
					std::byte{0x09},
					std::byte{0x0A},
					std::byte{0x0B},
					std::byte{0x0C},
					std::byte{0x0D},
					std::byte{0x0E},
					std::byte{0x0F}
				};
				Boring32::Crypto::CryptoKey key = aes.GenerateSymmetricKey(rgbAES128Key);

				std::vector<std::byte> rgbIV1 =
				{
					std::byte{0x00},
					std::byte{0x01},
					std::byte{0x02},
					std::byte{0x03},
					std::byte{0x04},
					std::byte{0x05},
					std::byte{0x06},
					std::byte{0x07},
					std::byte{0x08},
					std::byte{0x09},
					std::byte{0x0A},
					std::byte{0x0B},
					std::byte{0x0C},
					std::byte{0x0D},
					std::byte{0x0E},
					std::byte{0x0F}
				};
				// Copy the IV as it's modified during encryption
				std::vector<std::byte> rgbIV2 = rgbIV1;

				std::wstring testString = L"Hello, world";
				std::vector<std::byte> encrypted = aes.Encrypt(key, rgbIV1, testString);
				std::vector<std::byte> decrypted = aes.Decrypt(key, rgbIV2, encrypted);
				std::wstring decryptedString(
					(wchar_t*)&decrypted[0], 
					decrypted.size() / sizeof(wchar_t)
				);
				Assert::IsTrue(testString == decryptedString);
			}
	};
}
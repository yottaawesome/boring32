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
				aes.SetChainingMode(BCRYPT_CHAIN_MODE_CBC);
				aes.SetChainingMode(BCRYPT_CHAIN_MODE_CCM);
				aes.SetChainingMode(BCRYPT_CHAIN_MODE_CFB);
				aes.SetChainingMode(BCRYPT_CHAIN_MODE_ECB);
				aes.SetChainingMode(BCRYPT_CHAIN_MODE_GCM);
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

			TEST_METHOD(TestEncrypt)
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

				static std::vector<std::byte> rgbIV =
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

				std::wstring testString = L"Hello, world";
				aes.Encrypt(key, rgbIV, testString);
			}
	};
}
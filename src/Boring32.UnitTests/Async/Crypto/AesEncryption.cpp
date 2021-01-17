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
	};
}
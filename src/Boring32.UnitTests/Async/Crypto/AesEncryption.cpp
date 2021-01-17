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

			TEST_METHOD(TestObjectByteSize)
			{
				Boring32::Crypto::AesEncryption aes;
				Assert::IsTrue(aes.GetObjectByteSize() > 0);
			}
	};
}
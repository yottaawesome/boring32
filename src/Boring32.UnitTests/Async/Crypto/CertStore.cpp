#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Crypto/CertStore.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Crypto
{
	TEST_CLASS(CertStore)
	{
		public:
			TEST_METHOD(TestCreateCertStore)
			{
				Boring32::Crypto::CertStore store(L"MY");
				Assert::IsNotNull(store.GetHandle());
			}
	};
}
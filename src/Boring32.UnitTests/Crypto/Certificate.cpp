#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.crypto.certificate;
import boring32.crypto.certstore;

namespace Crypto
{
	TEST_CLASS(Certificate)
	{
		public:
			TEST_METHOD(TestGetCertificate)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert 
					= certStore.GetCertBySubjectCn(L"client.localhost");
				Assert::IsTrue(cert.GetSignatureHashCngAlgorithm() == L"RSA/SHA256");
			}
	};
}
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
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Crypto::Certificate cert;
				Assert::IsNull(cert.GetCert());
			}

			TEST_METHOD(TestCertLoad)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert
					= certStore.GetCertBySubjectCn(L"client.localhost");
				Assert::IsNotNull(cert.GetCert());
			}

			TEST_METHOD(TestGetSignatureHashCngAlgorithm)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert
					= certStore.GetCertBySubjectCn(L"client.localhost");
				Assert::IsTrue(cert.GetSignatureHashCngAlgorithm() == L"RSA/SHA256");
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2(cert1);
				Assert::IsNotNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Crypto::CertStore certStore(L"MY");
				Boring32::Crypto::Certificate cert1 = certStore.GetCertBySubjectCn(L"client.localhost");
				Boring32::Crypto::Certificate cert2 = cert1;
				Assert::IsNotNull(cert1.GetCert());
				Assert::IsNotNull(cert2.GetCert());
			}
	};
}
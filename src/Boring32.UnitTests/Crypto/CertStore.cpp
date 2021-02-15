#include "pch.h"
#include "CppUnitTest.h"
#include "Boring32/include/Crypto/CertStore.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Crypto
{
	TEST_CLASS(CertStore)
	{
		public:
			TEST_METHOD(TestCreateUserCertStore)
			{
				Boring32::Crypto::CertStore store(L"MY");
				Assert::IsNotNull(store.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store.GetStoreType()
				);
			}

			TEST_METHOD(TestCreateSystemCertStore)
			{
				Boring32::Crypto::CertStore store(L"MY", Boring32::Crypto::CertStoreType::System);
				Assert::IsNotNull(store.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::System == store.GetStoreType()
				);
			}

			TEST_METHOD(TestReadUserCert)
			{
				Boring32::Crypto::CertStore store(L"MY", Boring32::Crypto::CertStoreType::CurrentUser);
				std::wstring subject = L"client.localhost";
				//subject = L"CN = localhost";
				Assert::IsNotNull(
					store
						.FindBySubjectCn(subject)
						.GetCert()
				);
			}

			TEST_METHOD(TestReadSystemCert)
			{
				Boring32::Crypto::CertStore store(L"MY", Boring32::Crypto::CertStoreType::System);
				Assert::IsNotNull(
					store
						.GetCertByExactSubject(L"CN = localhost")
						.GetCert()
				);
			}

			TEST_METHOD(TestGetAll)
			{
				Boring32::Crypto::CertStore store(L"MY");
				std::vector<Boring32::Crypto::Certificate> certificates =
					store.GetAll();
				Assert::IsTrue(certificates.size());
			}
	};
}
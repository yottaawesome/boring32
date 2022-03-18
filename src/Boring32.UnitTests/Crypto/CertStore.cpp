#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
import boring32.crypto.certstore;

namespace Crypto
{
	TEST_CLASS(CertStore)
	{
		public:
			TEST_METHOD(TestDefaultConstructor)
			{
				Boring32::Crypto::CertStore store;
				Assert::IsTrue(store.GetHandle() == nullptr);
				Assert::IsTrue(store.GetStoreType() == Boring32::Crypto::CertStoreType::CurrentUser);
			}

			TEST_METHOD(TestCopyConstructor)
			{
				Boring32::Crypto::CertStore store1(L"MY");
				Boring32::Crypto::CertStore store2(store1);
				Assert::IsNotNull(store1.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store1.GetStoreType()
				);
				Assert::IsNotNull(store2.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store2.GetStoreType()
				);
			}

			TEST_METHOD(TestCopyAssignment)
			{
				Boring32::Crypto::CertStore store1(L"MY");
				Boring32::Crypto::CertStore store2 = store1;
				Assert::IsNotNull(store1.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store1.GetStoreType()
				);
				Assert::IsNotNull(store2.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store2.GetStoreType()
				);
			}

			TEST_METHOD(TestMoveConstructor)
			{
				Boring32::Crypto::CertStore store1(L"MY");
				Boring32::Crypto::CertStore store2(std::move(store1));
				Assert::IsNull(store1.GetHandle());
				Assert::IsNotNull(store2.GetHandle());
				Assert::IsTrue(
					Boring32::Crypto::CertStoreType::CurrentUser == store2.GetStoreType()
				);
			}

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
						.GetCertBySubjectCn(subject)
						.GetCert()
				);
			}

			TEST_METHOD(TestReadSystemCert)
			{
				Boring32::Crypto::CertStore store(L"MY", Boring32::Crypto::CertStoreType::System);
				Assert::IsNotNull(
					store
						.GetCertBySubjectCn(L"localhost")
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
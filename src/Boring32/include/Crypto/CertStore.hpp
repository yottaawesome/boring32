#pragma once
#include <string>
#include <Windows.h>
#include <wincrypt.h>

namespace Boring32::Crypto
{
	enum class CertStoreCloseOptions : DWORD
	{
		Default = 0,
		CheckNonFreedResources = CERT_CLOSE_STORE_CHECK_FLAG,
		ForceFreeMemory = CERT_CLOSE_STORE_FORCE_FLAG
	};

	class CertStore
	{
		public:
			virtual ~CertStore();
			CertStore();
			CertStore(HCERTSTORE certStore);
			CertStore(HCERTSTORE certStore, const CertStoreCloseOptions closeOptions);
			CertStore(std::wstring storeName);
			CertStore(std::wstring storeName, const CertStoreCloseOptions closeOptions);

			CertStore(const CertStore& other);
			virtual CertStore& operator=(const CertStore& other);

			CertStore(CertStore&& other) noexcept;
			virtual CertStore& operator=(CertStore&& other) noexcept;

		public:
			virtual void Close();
			virtual void Close(const std::nothrow_t&) noexcept;
			virtual HCERTSTORE GetHandle() const noexcept;
			virtual const std::wstring& GetName() const noexcept;
			virtual CERT_CONTEXT* GetCertBySubjectName(const std::wstring& subjectName);
			virtual CERT_CONTEXT* GetCertByIssuerName(const std::wstring& issuerName);

		protected:
			virtual void InternalOpen();
			virtual CertStore& Copy(const CertStore& other);
			virtual CertStore& Move(CertStore& other) noexcept;

		protected:
			HCERTSTORE m_certStore;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions;
	};
}
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

	enum class CertStoreType
	{
		CurrentUser,
		System
	};

	class CertStore
	{
		public:
			virtual ~CertStore();
			CertStore();
			CertStore(const HCERTSTORE certStore, const CertStoreType storeType);
			CertStore(
				const HCERTSTORE certStore, 
				const CertStoreType storeType, 
				const CertStoreCloseOptions closeOptions
			);
			CertStore(std::wstring storeName);
			CertStore(std::wstring storeName, const CertStoreType storeType);
			CertStore(
				std::wstring storeName,
				const CertStoreType storeType,
				const CertStoreCloseOptions closeOptions
			);

			CertStore(const CertStore& other);
			virtual CertStore& operator=(const CertStore& other);

			CertStore(CertStore&& other) noexcept;
			virtual CertStore& operator=(CertStore&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual HCERTSTORE GetHandle() const noexcept;
			virtual const std::wstring& GetName() const noexcept;
			virtual CERT_CONTEXT* GetCertBySubstringSubjectName(const std::wstring& subjectName);
			virtual CERT_CONTEXT* GetCertByExactSubjectName(const std::wstring& subjectName);
			virtual CERT_CONTEXT* GetCertBySubstringIssuerName(const std::wstring& issuerName);
			virtual CertStoreType GetStoreType() const noexcept;

		protected:
			virtual void InternalOpen();
			virtual CertStore& Copy(const CertStore& other);
			virtual CertStore& Move(CertStore& other) noexcept;
			virtual CERT_CONTEXT* GetCertByString(const DWORD searchFlag, const std::wstring& arg);

		protected:
			HCERTSTORE m_certStore;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions;
			CertStoreType m_storeType;
	};
}
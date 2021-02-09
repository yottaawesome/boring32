#pragma once
#include <string>
#include <Windows.h>
#include <wincrypt.h>
#include "Certificate.hpp"

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
			CertStore(CertStore&& other) noexcept;

		public:
			virtual CertStore& operator=(const CertStore& other);
			virtual CertStore& operator=(CertStore&& other) noexcept;
			virtual bool operator==(const CertStore& other) const noexcept;
			virtual operator bool() const noexcept;

		public:
			virtual void Close() noexcept;
			virtual HCERTSTORE GetHandle() const noexcept;
			virtual const std::wstring& GetName() const noexcept;
			virtual Certificate EnumerateAndFindBySubjectCn(const std::wstring& subjectCn);
			virtual Certificate GetCertBySubstringSubjectName(const std::wstring& subjectName);
			virtual Certificate GetCertByExactSubjectName(const std::wstring& subjectName);
			virtual Certificate GetCertBySubstringIssuerName(const std::wstring& issuerName);
			virtual CertStoreType GetStoreType() const noexcept;
			virtual void DeleteCert(const CERT_CONTEXT* cert);
			virtual void ImportCert(const CERT_CONTEXT* cert);

		protected:
			virtual void InternalOpen();
			virtual CertStore& Copy(const CertStore& other);
			virtual CertStore& Move(CertStore& other) noexcept;
			virtual CERT_CONTEXT* GetCertByArg(
				const DWORD searchFlag, 
				const void* arg
			);

		protected:
			HCERTSTORE m_certStore;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions;
			CertStoreType m_storeType;
	};
}
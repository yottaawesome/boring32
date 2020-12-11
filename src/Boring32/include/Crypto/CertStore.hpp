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
			CertStore(std::wstring storeName);
			CertStore(std::wstring storeName, const CertStoreCloseOptions closeOptions);

		public:
			virtual void Close();
			virtual void Close(const std::nothrow_t&) noexcept;
			virtual HCERTSTORE GetHandle() const noexcept;
			virtual const std::wstring& GetName() const noexcept;

		protected:
			virtual void InternalOpen();

		protected:
			HCERTSTORE m_certStore;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions;
	};
}
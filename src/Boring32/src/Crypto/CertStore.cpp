#include "pch.hpp"
#include <stdexcept>
#include <iostream>
#include "include/Error/Error.hpp"
#include "include/Crypto/CertStore.hpp"

namespace Boring32::Crypto
{
	CertStore::~CertStore()
	{
		Close(std::nothrow);
	}

	CertStore::CertStore()
		: m_certStore(nullptr),
		m_closeOptions(CertStoreCloseOptions::Default)
	{ }

	CertStore::CertStore(std::wstring storeName)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(CertStoreCloseOptions::Default)
	{
		InternalOpen();
	}

	CertStore::CertStore(
		std::wstring storeName,
		const CertStoreCloseOptions closeOptions
	)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(closeOptions)
	{
		InternalOpen();
	}

	CertStore::CertStore(const CertStore& other)
	:	m_certStore(nullptr)
	{
		Copy(other);
	}

	CertStore& CertStore::operator=(const CertStore& other)
	{
		return Copy(other);
	}

	CertStore& CertStore::Copy(const CertStore& other)
	{
		Close(std::nothrow);
		m_storeName = other.m_storeName;
		m_closeOptions = other.m_closeOptions;
		if (other.m_certStore)
			m_certStore = CertDuplicateStore(other.m_certStore);
		return *this;
	}

	CertStore::CertStore(CertStore&& other) noexcept
	:	m_certStore(nullptr)
	{
		Move(other);
	}

	CertStore& CertStore::operator=(CertStore&& other) noexcept
	{
		return Move(other);
	}
	
	CertStore& CertStore::Move(CertStore& other) noexcept
	{
		Close(std::nothrow);
		m_storeName = std::move(other.m_storeName);
		m_closeOptions = other.m_closeOptions;
		if (other.m_certStore)
		{
			m_certStore = other.m_certStore;
			other.m_certStore = nullptr;
		}
		return *this;
	}

	void CertStore::Close()
	{
		if (m_certStore)
		{
			// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certclosestore
			// for additional resource notes under remarks
			if (CertCloseStore(m_certStore, (DWORD)m_closeOptions) == false)
				throw Error::Win32Error(__FUNCSIG__ ": CertCloseStore() failed", GetLastError());
			m_certStore = nullptr;
		}
	}

	void CertStore::Close(const std::nothrow_t&) noexcept
	{
		try
		{
			Close();
		}
		catch (const std::exception& ex)
		{
			std::wcerr 
				<< __FUNCSIG__ 
				<< ": "
				<< "Close() failed"
				<< ex.what()
				<< std::endl;
		}
	}

	void CertStore::InternalOpen()
	{
		if (m_storeName.empty())
			throw std::invalid_argument(__FUNCSIG__ ": m_storeName is empty");
		// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certopensystemstorew
		// common store names: CA, MY, ROOT, SPC
		m_certStore = CertOpenSystemStoreW(0, m_storeName.c_str());

		/*
		HCERTSTORE CertOpenStore(
			LPCSTR            lpszStoreProvider,
			DWORD             dwEncodingType,
			HCRYPTPROV_LEGACY hCryptProv,
			DWORD             dwFlags,
			const void* pvPara
		);
		*/

		if (m_certStore == nullptr)
			throw Error::Win32Error(__FUNCSIG__ ": CertOpenSystemStoreW() failed", GetLastError());
	}

	HCERTSTORE CertStore::GetHandle() const noexcept
	{
		return m_certStore;
	}

	const std::wstring& CertStore::GetName() const noexcept
	{
		return m_storeName;
	}

	CERT_CONTEXT* CertStore::GetCertBySubjectName(const std::wstring& subjectName)
	{
		CERT_CONTEXT* certContext = (CERT_CONTEXT*)CertFindCertificateInStore(
			m_certStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING | CERT_FIND_HAS_PRIVATE_KEY,
			0,
			CERT_FIND_SUBJECT_STR,
			(void*)subjectName.c_str(), //Subject string in the certificate.
			nullptr
		);
		if (certContext == nullptr)
		{
			const DWORD lastError = GetLastError();
			if (lastError != CRYPT_E_NOT_FOUND)
				throw Error::Win32Error(__FUNCSIG__ ": CertFindCertificateInStore() failed", lastError);
		}
		return certContext;
	}

	CERT_CONTEXT* CertStore::GetCertByIssuerName(const std::wstring& issuerName)
	{
		CERT_CONTEXT* certContext = (CERT_CONTEXT*)CertFindCertificateInStore(
			m_certStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING | CERT_FIND_HAS_PRIVATE_KEY,
			0,
			CERT_FIND_ISSUER_STR,
			(void*)issuerName.c_str(), //Issuer name
			nullptr
		);
		if (certContext == nullptr)
		{
			const DWORD lastError = GetLastError();
			if (lastError != CRYPT_E_NOT_FOUND)
				throw Error::Win32Error(__FUNCSIG__ ": CertFindCertificateInStore() failed", lastError);
		}
		return certContext;
	}
}

#include "pch.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cryptuiapi.h>
#include "include/Error/Error.hpp"
#include "include/Crypto/CertStore.hpp"
#include "include/Strings/Strings.hpp"

namespace Boring32::Crypto
{
	CertStore::~CertStore()
	{
		Close();
	}

	CertStore::CertStore()
	:	m_certStore(nullptr),
		m_closeOptions(CertStoreCloseOptions::Default),
		m_storeType(CertStoreType::CurrentUser)
	{ }

	CertStore::CertStore(const HCERTSTORE certStore, const CertStoreType storeType)
	:	m_certStore(certStore),
		m_closeOptions(CertStoreCloseOptions::Default),
		m_storeType(storeType)
	{ }

	CertStore::CertStore(
		const HCERTSTORE certStore,
		const CertStoreType storeType,
		const CertStoreCloseOptions closeOptions
	)
	:	m_certStore(certStore),
		m_closeOptions(closeOptions),
		m_storeType(storeType)
	{ }

	CertStore::CertStore(std::wstring storeName)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(CertStoreCloseOptions::Default),
		m_storeType(CertStoreType::CurrentUser)
	{
		InternalOpen();
	}

	CertStore::CertStore(std::wstring storeName, const CertStoreType storeType)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(CertStoreCloseOptions::Default),
		m_storeType(storeType)
	{
		InternalOpen();
	}

	CertStore::CertStore(
		std::wstring storeName,
		const CertStoreType storeType,
		const CertStoreCloseOptions closeOptions
	)
	:	m_certStore(nullptr),
		m_storeName(std::move(storeName)),
		m_closeOptions(closeOptions),
		m_storeType(storeType)
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
		Close();
		m_storeName = other.m_storeName;
		m_closeOptions = other.m_closeOptions;
		m_storeType = other.m_storeType;
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

	bool CertStore::operator==(const CertStore& other) const noexcept
	{
		return m_certStore == other.m_certStore;
	}

	CertStore::operator bool() const noexcept
	{
		return m_certStore != nullptr;
	}
	
	CertStore& CertStore::Move(CertStore& other) noexcept
	{
		Close();
		m_storeName = std::move(other.m_storeName);
		m_closeOptions = other.m_closeOptions;
		m_storeType = other.m_storeType;
		if (other.m_certStore)
		{
			m_certStore = other.m_certStore;
			other.m_certStore = nullptr;
		}
		return *this;
	}

	void CertStore::Close() noexcept
	{
		if (m_certStore)
		{
			// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certclosestore
			// for additional resource notes under remarks
			if (CertCloseStore(m_certStore, (DWORD)m_closeOptions) == false)
			{
				Error::Win32Error error(__FUNCSIG__ ": CertCloseStore() failed", GetLastError());
				std::wcerr << error.what() << std::endl;
			}
			m_certStore = nullptr;
		}
	}

	void CertStore::InternalOpen()
	{
		if (m_storeName.empty())
			throw std::invalid_argument(__FUNCSIG__ ": m_storeName is empty");
		
		switch (m_storeType)
		{
			case CertStoreType::CurrentUser:
			{
				// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certopensystemstorew
				// common store names: CA, MY, ROOT, SPC
				m_certStore = CertOpenSystemStoreW(0, m_storeName.c_str());
				break;
			}

			case CertStoreType::System:
			{
				m_certStore = CertOpenStore(
					CERT_STORE_PROV_SYSTEM_REGISTRY_W,
					PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
					0,
					CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE,
					m_storeName.c_str()
				);
				break;
			}

			default:
				throw std::runtime_error(__FUNCSIG__ ": unknown m_storeType");
		}
		
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

	Certificate CertStore::EnumerateAndFindBySubjectCn(const std::wstring& subjectCn)
	{
		PCCERT_CONTEXT currentCert = nullptr;
		Certificate cert;
		while (currentCert = CertEnumCertificatesInStore(m_certStore, currentCert))
		{
			cert.Attach(currentCert);
			std::wstring name = cert.GetFormattedSubjectName(CERT_X500_NAME_STR);
			std::vector<std::wstring> tokens = Strings::TokeniseString(name, L", ");
			for (const std::wstring& token : tokens)
			{
				if (token.starts_with(L"CN="))
				{
					std::wstring cleanedName = Strings::Replace(token, L"CN=", L"");
					if (subjectCn == cleanedName)
						return cert;
				}
			}

			// The cert is automatically freed by the next call to CertEnumCertificatesInStore
			// We only use Certificate to provide us with exception-based clean up and to use
			// GetFormattedSubjectName()
			cert.Detach();
		}
		const DWORD lastError = GetLastError();
		if (lastError != CRYPT_E_NOT_FOUND && lastError != ERROR_NO_MORE_FILES)
			throw Error::Win32Error(__FUNCSIG__ ": CertEnumCertificatesInStore() failed", lastError);
		return nullptr;
	}

	Certificate CertStore::GetCertByExactSubjectName(const std::wstring& subjectName)
	{
		DWORD encoded = 0;
		bool succeeded = CertStrToNameW(
			X509_ASN_ENCODING,
			subjectName.c_str(),
			CERT_OID_NAME_STR,
			nullptr,
			nullptr,
			&encoded,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__, GetLastError());

		std::vector<BYTE> byte(encoded, 0);
		succeeded = CertStrToNameW(
			X509_ASN_ENCODING,
			subjectName.c_str(),
			CERT_OID_NAME_STR,
			nullptr,
			&byte[0],
			&encoded,
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__, GetLastError());
		
		CERT_NAME_BLOB blob{
			.cbData = (DWORD)byte.size(),
			.pbData = &byte[0]
		};
		return GetCertByArg(CERT_FIND_SUBJECT_NAME, &blob);
	}

	Certificate CertStore::GetCertBySubstringSubjectName(const std::wstring& subjectName)
	{
		return GetCertByArg(CERT_FIND_SUBJECT_STR, subjectName.c_str());
	}

	Certificate CertStore::GetCertBySubstringIssuerName(const std::wstring& issuerName)
	{
		return GetCertByArg(CERT_FIND_ISSUER_STR, issuerName.c_str());
	}

	CERT_CONTEXT* CertStore::GetCertByArg(const DWORD searchFlag, const void* arg)
	{
		CERT_CONTEXT* const certContext = (CERT_CONTEXT*)CertFindCertificateInStore(
			m_certStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING | CERT_FIND_HAS_PRIVATE_KEY,
			0,
			searchFlag,
			(void*)arg,
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

	CertStoreType CertStore::GetStoreType() const noexcept
	{
		return m_storeType;
	}

	void CertStore::DeleteCert(const CERT_CONTEXT* cert)
	{
		if (cert == nullptr)
			throw std::invalid_argument("cert is nullptr");
		if (m_certStore == nullptr)
			throw std::runtime_error("m_certStore is nullptr");

		if (CertDeleteCertificateFromStore(cert) == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CertDeleteCertificateFromStore() failed",
				GetLastError()
			);
	}

	void CertStore::ImportCert(const CERT_CONTEXT* cert)
	{
		if (cert == nullptr)
			throw std::invalid_argument("cert is nullptr");
		if (m_certStore == nullptr)
			throw std::runtime_error("m_certStore is nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_import_src_info
		CRYPTUI_WIZ_IMPORT_SRC_INFO info{
			.dwSize = sizeof(info),
			.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT,
			.pCertContext = cert,
			.dwFlags = 0,
			.pwszPassword = L""
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizimport
		bool succeeded = CryptUIWizImport(
			CRYPTUI_WIZ_NO_UI,
			nullptr,
			nullptr,
			&info,
			m_certStore
		);
		if(succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CryptUIWizImport() failed",
				GetLastError()
			);
	}
}

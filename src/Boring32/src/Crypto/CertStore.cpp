#include "pch.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cryptuiapi.h>
#include "include/Error/Error.hpp"
#include "include/Crypto/CryptoFuncs.hpp"
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
		if (m_storeType != CertStoreType::InMemory && m_storeName.empty())
			throw std::invalid_argument(__FUNCSIG__ ": m_storeName is required for non-memory stores");
		
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

			case CertStoreType::InMemory:
			{
				m_certStore = CertOpenStore(
					CERT_STORE_PROV_MEMORY,
					PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
					0,
					CERT_SYSTEM_STORE_CURRENT_USER,
					nullptr
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

	std::vector<Certificate> CertStore::GetAll()
	{
		std::vector<Certificate> results;
		PCCERT_CONTEXT currentCert = nullptr;
		// The cert is automatically freed by the next call to CertEnumCertificatesInStore
		while (currentCert = CertEnumCertificatesInStore(m_certStore, currentCert))
			results.emplace_back(currentCert, false);
		
		const DWORD lastError = GetLastError();
		if (lastError != CRYPT_E_NOT_FOUND && lastError != ERROR_NO_MORE_FILES)
			throw Error::Win32Error(__FUNCSIG__ ": CertEnumCertificatesInStore() failed", lastError);

		return results;
	}

	Certificate CertStore::GetCertByFormattedSubject(const std::wstring& subjectRdn)
	{
		PCCERT_CONTEXT currentCert = nullptr;
		Certificate cert;
		while (currentCert = CertEnumCertificatesInStore(m_certStore, currentCert))
		{
			cert.Attach(currentCert);
			std::wstring name = cert.GetFormattedSubject(CERT_X500_NAME_STR);
			if (name == subjectRdn)
				return cert;

			// The cert is automatically freed by the next call to CertEnumCertificatesInStore
			// We only use Certificate to provide us with exception-based clean up and to use
			// GetFormattedSubjectName()
			cert.Detach();
		}
		const DWORD lastError = GetLastError();
		if (lastError != CRYPT_E_NOT_FOUND && lastError != ERROR_NO_MORE_FILES)
			throw Error::Win32Error(__FUNCSIG__ ": CertEnumCertificatesInStore() failed", lastError);

		return Certificate();
	}

	Certificate CertStore::GetCertBySubjectCn(const std::wstring& subjectCn)
	{
		PCCERT_CONTEXT currentCert = nullptr;
		Certificate cert;
		while (currentCert = CertEnumCertificatesInStore(m_certStore, currentCert))
		{
			cert.Attach(currentCert);
			std::wstring name = cert.GetFormattedSubject(CERT_X500_NAME_STR);
			std::vector<std::wstring> tokens = Strings::TokeniseString(name, L", ");
			for (const std::wstring& token : tokens)
				if (token.starts_with(L"CN="))
					if (subjectCn == Strings::Replace(token, L"CN=", L""))
						return cert;

			// The cert is automatically freed by the next call to CertEnumCertificatesInStore
			// We only use Certificate to provide us with exception-based clean up and to use
			// GetFormattedSubjectName()
			cert.Detach();
		}
		const DWORD lastError = GetLastError();
		if (lastError != CRYPT_E_NOT_FOUND && lastError != ERROR_NO_MORE_FILES)
			throw Error::Win32Error(__FUNCSIG__ ": CertEnumCertificatesInStore() failed", lastError);

		return Certificate();
	}

	Certificate CertStore::GetCertByExactSubject(const std::vector<std::byte>& subjectName)
	{
		CERT_NAME_BLOB blob{
			.cbData = (DWORD)subjectName.size(),
			.pbData = (BYTE*)&subjectName[0]
		};
		return GetCertByArg(CERT_FIND_SUBJECT_NAME, &blob);
	}

	/*
	Certificate CertStore::GetCertByExactSubjectRdn(const std::string& subjectName)
	{
		DWORD cbEncoded = 0;
		CERT_RDN_ATTR rgNameAttr =
		{
		   (LPSTR)szOID_COMMON_NAME,                // the OID
		   CERT_RDN_PRINTABLE_STRING,        // type of string
		   (DWORD)subjectName.size() + 1,   // string length including
											 // the terminating null 
											 // character
		   (BYTE*)subjectName.c_str()            // pointer to the string
		};
		CERT_RDN rgRDN[] =
		{
		   1,               // the number of elements in the array
		   &rgNameAttr      // pointer to the array
		};
		CERT_NAME_INFO CertName =
		{
			1,          // number of elements in the CERT_RND's array
			rgRDN
		};

		CryptEncodeObjectEx(
			X509_ASN_ENCODING,        // the encoding/decoding type
			X509_NAME,
			&CertName,
			0,
			NULL,
			NULL,
			&cbEncoded
		);

		std::vector<std::byte> pbEncoded(cbEncoded);
		pbEncoded.resize(cbEncoded);

		CryptEncodeObjectEx(
			X509_ASN_ENCODING,
			X509_NAME,
			&CertName,
			0,
			NULL,
			(BYTE*)&pbEncoded[0],
			&cbEncoded
		);

		CERT_NAME_BLOB blob{
			.cbData = (DWORD)pbEncoded.size(),
			.pbData = (BYTE*)&pbEncoded[0]
		};
		return GetCertByArg(CERT_FIND_SUBJECT_NAME, &blob);
	}
	*/
	
	Certificate CertStore::GetCertByExactSubject(const std::wstring& subjectName)
	{
		std::vector<std::byte> encodedBytes = EncodeAsnString(subjectName);
		const CERT_NAME_BLOB blob{
			.cbData = (DWORD)encodedBytes.size(),
			.pbData = (BYTE*)&encodedBytes[0]
		};
		return GetCertByArg(CERT_FIND_SUBJECT_NAME, &blob);
	}

	Certificate CertStore::GetCertByExactIssuer(const std::wstring& subjectName)
	{
		std::vector<std::byte> encodedBytes = EncodeAsnString(subjectName);
		const CERT_NAME_BLOB blob{
			.cbData = (DWORD)encodedBytes.size(),
			.pbData = (BYTE*)&encodedBytes[0]
		};
		return GetCertByArg(CERT_FIND_ISSUER_NAME, &blob);
	}

	Certificate CertStore::GetCertBySubstringSubject(const std::wstring& subjectName)
	{
		return GetCertByArg(CERT_FIND_SUBJECT_STR, subjectName.c_str());
	}

	Certificate CertStore::GetCertBySubstringIssuerName(const std::wstring& issuerName)
	{
		return GetCertByArg(CERT_FIND_ISSUER_STR, issuerName.c_str());
	}

	Certificate CertStore::GetCertByByBase64Signature(const std::wstring& base64Signature)
	{
		std::vector<std::byte> bytes = ToBinary(base64Signature);
		CRYPT_HASH_BLOB blob{
			.cbData = (DWORD)bytes.size(),
			.pbData = (BYTE*)&bytes[0]
		};
		return GetCertByArg(CERT_FIND_SIGNATURE_HASH, &blob);
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
			throw std::invalid_argument(__FUNCSIG__ ": cert is nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_import_src_info
		CRYPTUI_WIZ_IMPORT_SRC_INFO info{
			.dwSize = sizeof(info),
			.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT,
			.pCertContext = cert,
			.dwFlags = 0,
			.pwszPassword = L""
		};
		InternalImport(info);
	}

	void CertStore::ImportCertsFromFile(const std::filesystem::path& path, const std::wstring& password)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_import_src_info
		std::wstring resolvedAbsolutePath = std::filesystem::absolute(path);
		CRYPTUI_WIZ_IMPORT_SRC_INFO info{
			.dwSize = sizeof(info),
			.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE,
			.pwszFileName = resolvedAbsolutePath.c_str(),
			.dwFlags = 0,
			.pwszPassword = password.c_str()
		};
		InternalImport(info);
	}

	void CertStore::AddCertificate(const CERT_CONTEXT* cert)
	{
		if (cert == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": cert is null");
		if (m_certStore == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_certStore is nullptr");
		const bool succeeded = CertAddCertificateContextToStore(
			m_certStore,
			cert, 
			CERT_STORE_ADD_REPLACE_EXISTING, 
			nullptr
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CertAddCertificateContextToStore()", 
				GetLastError()
			);
	}

	void CertStore::InternalImport(const CRYPTUI_WIZ_IMPORT_SRC_INFO& info)
	{
		if (m_certStore == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_certStore is nullptr");

		const static DWORD CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS = 0x0002;

		// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/nf-cryptuiapi-cryptuiwizimport
		bool succeeded = CryptUIWizImport(
			CRYPTUI_WIZ_NO_UI | CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS | CRYPTUI_WIZ_IMPORT_ALLOW_CERT,
			nullptr,
			nullptr,
			&info,
			m_certStore
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CryptUIWizImport() failed",
				GetLastError()
			);
	}
}

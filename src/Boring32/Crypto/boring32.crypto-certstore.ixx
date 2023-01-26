//#include <Windows.h>
//#include <dpapi.h> // not including this header causes symbol has already been defined error
//#include <wincrypt.h>
//#include <cryptuiapi.h>
//#include <winnt.h>

export module boring32.crypto:certstore;
import :certificate;
import <string>;
import <filesystem>;
import <vector>;
import <win32.hpp>;

export namespace Boring32::Crypto
{
	enum class CertStoreCloseOptions : DWORD
	{
		Default = 0,
		CheckNonFreedResources = CERT_CLOSE_STORE_CHECK_FLAG,
		ForceFreeMemory = CERT_CLOSE_STORE_FORCE_FLAG
	};
	DEFINE_ENUM_FLAG_OPERATORS(CertStoreCloseOptions);

	enum class CertStoreType
	{
		CurrentUser,
		System,
		InMemory
	};

	class CertStore
	{
		public:
			virtual ~CertStore();
			CertStore() = default;
			CertStore(const CertStore& other);
			CertStore(CertStore&& other) noexcept;
		
		public:
			CertStore(std::wstring storeName);
			CertStore(
				std::wstring storeName, 
				const CertStoreType storeType
			);
			CertStore(
				const HCERTSTORE certStore, 
				const CertStoreType storeType,
				const bool ownedExclusively
			);
			CertStore(
				const HCERTSTORE certStore, 
				const CertStoreType storeType, 
				const bool ownedExclusively,
				const CertStoreCloseOptions closeOptions
			);
			CertStore(
				std::wstring storeName,
				const CertStoreType storeType,
				const CertStoreCloseOptions closeOptions
			);

		public:
			virtual CertStore& operator=(const CertStore& other);
			virtual CertStore& operator=(CertStore&& other) noexcept;
			virtual bool operator==(const CertStore& other) const noexcept;
			virtual operator bool() const noexcept;

		public:
			virtual void Close() noexcept;
			virtual [[nodiscard]] HCERTSTORE GetHandle() const noexcept;
			virtual [[nodiscard]] const std::wstring& GetName() const noexcept;
			virtual [[nodiscard]] std::vector<Certificate> GetAll() const;
			virtual [[nodiscard]] Certificate GetCertByFormattedSubject(
				const std::wstring& subjectRdn
			) const;
			virtual [[nodiscard]] Certificate GetCertBySubjectCn(
				const std::wstring& subjectCn
			) const;
			virtual [[nodiscard]] Certificate GetCertBySubstringSubject(
				const std::wstring& subjectName
			) const;
			virtual [[nodiscard]] Certificate GetCertByExactSubject(
				const std::wstring& subjectName
			) const;
			virtual [[nodiscard]] Certificate GetCertByExactSubject(
				const std::vector<std::byte>& subjectName
			) const;
			virtual [[nodiscard]] Certificate GetCertByExactIssuer(
				const std::wstring& subjectName
			) const;
			virtual [[nodiscard]] Certificate GetCertBySubstringIssuerName(
				const std::wstring& issuerName
			) const;
			virtual [[nodiscard]] Certificate GetCertByByBase64Signature(
				const std::wstring& thumbprint
			) const;
			virtual [[nodiscard]] CertStoreType GetStoreType() const noexcept;
			// Note that this function frees the cert. Increase the
			// cert's reference count if this is not the wanted
			// behaviour.
			virtual void DeleteCert(const CERT_CONTEXT* cert);
			virtual void ImportCert(const CERT_CONTEXT* cert);
			virtual void ImportCertsFromFile(
				const std::filesystem::path& path, 
				const std::wstring& password
			);
			virtual void AddCertificate(const CERT_CONTEXT* cert);

		public:
			//virtual Certificate GetCertByExactSubjectRdn(const std::string& subjectName);

		protected:
			virtual CertStore& Copy(const CertStore& other);
			virtual CertStore& Move(CertStore& other) noexcept;
			virtual void InternalOpen();
			virtual PCCERT_CONTEXT GetCertByArg(
				const DWORD searchFlag, 
				const void* arg
			) const;

		protected:
			HCERTSTORE m_certStore = nullptr;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions = CertStoreCloseOptions::Default;
			CertStoreType m_storeType = CertStoreType::CurrentUser;
	};
}
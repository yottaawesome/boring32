module;

//#include <Windows.h>
//#include <dpapi.h> // not including this header causes symbol has already been defined error
//#include <wincrypt.h>
//#include <cryptuiapi.h>
//#include <winnt.h>

export module boring32.crypto:certstore;
import boring32.shared;
import boring32.strings;
import boring32.error;
import :functions;
import :certificate;

export namespace Boring32::Crypto
{
	using CertAddDisposition = Win32::CertAddDisposition;
	using CertStoreCloseOptions = Win32::CertStoreCloseOptions;

	enum class CertStoreType
	{
		CurrentUser,
		System,
		InMemory
	};

	class CertStore final
	{
		// The Six
		public:
			~CertStore()
			{
				Close();
			}

			CertStore() = default;

			CertStore(const CertStore& other)
			{
				Copy(other);
			}

			CertStore& operator=(const CertStore& other)
			{
				return Copy(other);
			}

			CertStore(CertStore&& other) noexcept
				: m_certStore(nullptr)
			{
				Move(other);
			}

			CertStore& operator=(CertStore&& other) noexcept
			{
				return Move(other);
			}
		
		public:
			CertStore(std::wstring storeName)
				: m_storeName(std::move(storeName))
			{
				InternalOpen();
			}

			CertStore(
				std::wstring storeName, 
				const CertStoreType storeType
			) : m_storeName(std::move(storeName)),
				m_storeType(storeType)
			{
				InternalOpen();
			}

			CertStore(
				const Win32::HCERTSTORE certStore, 
				const CertStoreType storeType,
				const bool ownedExclusively
			) : m_certStore(certStore),
				m_storeType(storeType)
			{
				if (certStore)
				{
					m_certStore = ownedExclusively
						? certStore
						: Win32::CertDuplicateStore(certStore);
				}
			}

			CertStore(
				const Win32::HCERTSTORE certStore,
				const CertStoreType storeType, 
				const bool ownedExclusively,
				const CertStoreCloseOptions closeOptions
			) : m_certStore(certStore),
				m_closeOptions(closeOptions),
				m_storeType(storeType)
			{
				if (certStore)
				{
					m_certStore = ownedExclusively
						? certStore
						: Win32::CertDuplicateStore(certStore);
				}
			}

			CertStore(
				std::wstring storeName,
				const CertStoreType storeType,
				const CertStoreCloseOptions closeOptions
			) : m_storeName(std::move(storeName)),
				m_closeOptions(closeOptions),
				m_storeType(storeType)
			{
				InternalOpen();
			}

		public:
			bool operator==(const CertStore& other) const noexcept
			{
				return m_certStore == other.m_certStore;
			}

			operator bool() const noexcept
			{
				return m_certStore != nullptr;
			}

		public:
			void Close() noexcept
			{
				if (!m_certStore)
					return;
				// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certclosestore
				// for additional resource notes under remarks
				if (!Win32::CertCloseStore(m_certStore, (Win32::DWORD)m_closeOptions))
				{
					const auto lastError = Win32::GetLastError();
					Error::Win32Error error("CertCloseStore() failed", lastError);
					// ICEs -- disabled for now
					std::wcerr << error.what() << std::endl;
				}
				m_certStore = nullptr;
			}

			[[nodiscard]] Win32::HCERTSTORE GetHandle() const noexcept
			{
				return m_certStore;
			}

			[[nodiscard]] const std::wstring& GetName() const noexcept
			{
				return m_storeName;
			}

			[[nodiscard]] std::vector<Certificate> GetAll() const
			{
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is null");

				std::vector<Certificate> results;
				Win32::PCCERT_CONTEXT currentCert = nullptr;
				// The cert is automatically freed by the next call to CertEnumCertificatesInStore
				// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certenumcertificatesinstore
				while (currentCert = Win32::CertEnumCertificatesInStore(m_certStore, currentCert))
				{
					// I'm not sure if this intermediate error check is necessary.
					// The MSDN sample doesn't do an error check, but I've added it
					// just in case.
					const Win32::DWORD lastError = Win32::GetLastError();
					if (lastError != Win32::ErrorCodes::FileNotFound)
						throw Error::Win32Error(
							"CertEnumCertificatesInStore() failed", 
							lastError
						);
					results.emplace_back(currentCert, false);
				}
				const DWORD lastError = Win32::GetLastError();
				if (lastError != Win32::CryptoErrorCodes::NotFound && lastError != Win32::ErrorCodes::NoMoreFiles)
					throw Error::Win32Error("CertEnumCertificatesInStore() failed", lastError);

				return results;
			}

			[[nodiscard]] Certificate GetCertByFormattedSubject(
				const std::wstring& subjectRdn
			) const
			{
				Win32::PCCERT_CONTEXT currentCert = nullptr;
				Certificate cert;
				while (currentCert = Win32::CertEnumCertificatesInStore(m_certStore, currentCert))
				{
					cert.Attach(currentCert);
					std::wstring name = cert.GetFormattedSubject(Win32::CertX500NameStr);
					if (name == subjectRdn)
						return cert;

					// The cert is automatically freed by the next call to CertEnumCertificatesInStore
					// We only use Certificate to provide us with exception-based clean up and to use
					// GetFormattedSubjectName()
					auto dummy = cert.Detach();
				}
				const Win32::DWORD lastError = Win32::GetLastError();
				if (lastError != Win32::CryptoErrorCodes::NotFound && lastError != Win32::ErrorCodes::NoMoreFiles)
					throw Error::Win32Error("CertEnumCertificatesInStore() failed", lastError);

				return {};
			}

			[[nodiscard]] Certificate GetCertBySubjectCn(
				const std::wstring& subjectCn
			) const
			{
				Win32::PCCERT_CONTEXT currentCert = nullptr;
				while (currentCert = CertEnumCertificatesInStore(m_certStore, currentCert))
				{
					// The cert is automatically freed by the next call to CertEnumCertificatesInStore
					// but Certificate also does its own cleanup, so signal that it needs to duplicate
					// the handle
					const Certificate cert(currentCert, false);
					const std::wstring name = cert.GetFormattedSubject(Win32::CertX500NameStr);
					const std::vector<std::wstring> tokens = Strings::TokeniseString(name, L", ");
					for (const std::wstring& token : tokens)
						if (token.starts_with(L"CN="))
							if (subjectCn == Strings::Replace(token, L"CN=", L""))
								return cert;
				}
				const Win32::DWORD lastError = Win32::GetLastError();
				if (lastError != Win32::CryptoErrorCodes::NotFound && lastError != Win32::ErrorCodes::NoMoreFiles)
					throw Error::Win32Error("CertEnumCertificatesInStore() failed", lastError);

				return {};
			}

			[[nodiscard]] Certificate GetCertBySubstringSubject(
				const std::wstring& subjectName
			) const
			{
				return { GetCertByArg(StoreFindType::SubjectStr, subjectName.c_str()), true };
			}

			[[nodiscard]] Certificate GetExisting(
				const Win32::PCCERT_CONTEXT cert
			) const
			{
				return { GetCertByArg(StoreFindType::Existing, cert), true };
			}

			[[nodiscard]] Certificate GetCertByExactSubject(
				const std::wstring& subjectName
			) const
			{
				std::vector<std::byte> encodedBytes = EncodeAsnString(subjectName);
				const Win32::CERT_NAME_BLOB blob{
					.cbData = static_cast<Win32::DWORD>(encodedBytes.size()),
					.pbData = reinterpret_cast<Win32::BYTE*>(&encodedBytes[0])
				};
				return { GetCertByArg(StoreFindType::SubjectName, &blob), true };
			}

			[[nodiscard]] Certificate GetCertByExactSubject(
				const std::vector<std::byte>& subjectName
			) const
			{
				Win32::CERT_NAME_BLOB blob{
					.cbData = static_cast<Win32::DWORD>(subjectName.size()),
					.pbData = reinterpret_cast<Win32::BYTE*>(const_cast<std::byte*>(&subjectName[0]))
				};
				return { GetCertByArg(StoreFindType::SubjectName, &blob), true };
			}

			[[nodiscard]] Certificate GetCertByExactIssuer(
				const std::wstring& subjectName
			) const
			{
				std::vector<std::byte> encodedBytes = EncodeAsnString(subjectName);
				const Win32::CERT_NAME_BLOB blob{
					.cbData = static_cast<Win32::DWORD>(encodedBytes.size()),
					.pbData = reinterpret_cast<Win32::BYTE*>(&encodedBytes[0])
				};
				return { GetCertByArg(StoreFindType::IssuerName, &blob), true };
			}

			[[nodiscard]] Certificate GetCertBySubstringIssuerName(
				const std::wstring& issuerName
			) const
			{
				return { GetCertByArg(StoreFindType::IssuerStr, issuerName.c_str()), true };
			}

			[[nodiscard]] Certificate GetCertByByBase64Signature(
				const std::wstring& base64Signature
			) const
			{
				std::vector<std::byte> bytes = ToBinary(base64Signature);
				const Win32::CRYPT_HASH_BLOB blob{
					.cbData = static_cast<Win32::DWORD>(bytes.size()),
					.pbData = reinterpret_cast<Win32::BYTE*>(&bytes[0])
				};
				return { GetCertByArg(StoreFindType::SignatureHash, &blob), true };
			}

			[[nodiscard]] CertStoreType GetStoreType() const noexcept
			{
				return m_storeType;
			}

			// Note that this function frees the cert. Increase the
			// cert's reference count if this is not the wanted
			// behaviour.
			void DeleteCert(const Win32::CERT_CONTEXT* cert)
			{
				if (!cert)
					throw Error::Boring32Error("cert is nullptr");
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certdeletecertificatefromstore
				if (!Win32::CertDeleteCertificateFromStore(cert))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"CertDeleteCertificateFromStore() failed",
						lastError
					);
				}
			}

			void ImportCert(const Win32::CERT_CONTEXT* cert)
			{
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is nullptr");
				if (!cert)
					throw Error::Boring32Error("cert is nullptr");

				// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_import_src_info
				Win32::CRYPTUI_WIZ_IMPORT_SRC_INFO info{
					.dwSize = sizeof(info),
					.dwSubjectChoice = Win32::_CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT,
					.pCertContext = cert,
					.dwFlags = 0,
					.pwszPassword = L""
				};
				ImportCertToStore(m_certStore, info);
			}

			void ImportCertsFromFile(
				const std::filesystem::path& path, 
				const std::wstring& password
			)
			{
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is nullptr");
				// https://docs.microsoft.com/en-us/windows/win32/api/cryptuiapi/ns-cryptuiapi-cryptui_wiz_import_src_info
				const std::wstring resolvedAbsolutePath = std::filesystem::absolute(path);
				Win32::CRYPTUI_WIZ_IMPORT_SRC_INFO info{
					.dwSize = sizeof(info),
					.dwSubjectChoice = Win32::_CRYPTUI_WIZ_IMPORT_SUBJECT_FILE,
					.pwszFileName = resolvedAbsolutePath.c_str(),
					.dwFlags = 0,
					.pwszPassword = password.c_str()
				};
				ImportCertToStore(m_certStore, info);
			}

			void AddCertificate(const Win32::CERT_CONTEXT* cert, const CertAddDisposition disposition)
			{
				if (!cert)
					throw Error::Boring32Error("cert is null");
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certaddcertificatecontexttostore
				const bool succeeded = Win32::CertAddCertificateContextToStore(
					m_certStore,
					cert,
					static_cast<Win32::DWORD>(disposition),
					nullptr
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"CertAddCertificateContextToStore() failed",
						lastError
					);
				}
			}

			Win32::PCCERT_CONTEXT GetCertByArg(
				const StoreFindType searchFlag,
				const void* arg
			) const
			{
				return Crypto::GetCertByArg(
					m_certStore,
					searchFlag,
					arg
				);
			}

			std::wstring GetLocalisedName() const
			{
				if (!m_certStore)
					throw Error::Boring32Error("m_certStore is nullptr");

				Win32::DWORD bytes = 0;
				bool succeeded = Win32::CertGetStoreProperty(
					m_certStore,
					Win32::_CERT_STORE_LOCALIZED_NAME_PROP_ID,
					nullptr,
					&bytes
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"CertGetStoreProperty() failed",
						lastError
					);
				}

				std::wstring returnValue(bytes / sizeof(wchar_t), '\0');
				succeeded = Win32::CertGetStoreProperty(
					m_certStore,
					Win32::_CERT_STORE_LOCALIZED_NAME_PROP_ID,
					&returnValue[0],
					&bytes
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error(
						"CertGetStoreProperty() failed",
						lastError
					);
				}
				returnValue.resize(bytes / sizeof(wchar_t));
				if (returnValue.ends_with('\0'))
					returnValue.pop_back();

				return returnValue;
			}

		public:
			//virtual Certificate GetCertByExactSubjectRdn(const std::string& subjectName);

		private:
			CertStore& Copy(const CertStore& other)
			{
				if (this == &other)
					return *this;

				Close();
				m_storeName = other.m_storeName;
				m_closeOptions = other.m_closeOptions;
				m_storeType = other.m_storeType;
				if (other.m_certStore)
					m_certStore = Win32::CertDuplicateStore(other.m_certStore);
				return *this;
			}

			CertStore& Move(CertStore& other) noexcept
			{
				if (this == &other)
					return *this;

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

			void InternalOpen()
			{
				if (m_storeType != CertStoreType::InMemory && m_storeName.empty())
					throw Error::Boring32Error("m_storeName is required for non-memory stores");

				switch (m_storeType)
				{
					case CertStoreType::CurrentUser:
					{
						// See https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certopensystemstorew
						// common store names: CA, MY, ROOT, SPC
						m_certStore = Win32::CertOpenSystemStoreW(0, m_storeName.c_str());
						break;
					}

					case CertStoreType::System:
					{
						m_certStore = Win32::CertOpenStore(
							Win32::_CERT_STORE_PROV_SYSTEM_REGISTRY_W,
							Win32::Pkcs7AsnEncoding | Win32::X509AsnEncoding,
							0,
							Win32::_CERT_STORE_OPEN_EXISTING_FLAG | Win32::_CERT_SYSTEM_STORE_LOCAL_MACHINE,
							m_storeName.c_str()
						);
						break;
					}

					case CertStoreType::InMemory:
					{
						m_certStore = Win32::CertOpenStore(
							Win32::_CERT_STORE_PROV_MEMORY,
							Win32::Pkcs7AsnEncoding | Win32::X509AsnEncoding,
							0,
							Win32::_CERT_SYSTEM_STORE_CURRENT_USER,
							nullptr
						);
						break;
					}

					default:
						throw Error::Boring32Error("unknown m_storeType");
				}

				if (!m_certStore)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("CertOpenSystemStoreW() failed", lastError);
				}
			}

		private:
			Win32::HCERTSTORE m_certStore = nullptr;
			std::wstring m_storeName;
			CertStoreCloseOptions m_closeOptions = CertStoreCloseOptions::Default;
			CertStoreType m_storeType = CertStoreType::CurrentUser;
	};

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
}
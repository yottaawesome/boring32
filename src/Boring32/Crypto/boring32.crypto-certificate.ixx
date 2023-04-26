export module boring32.crypto:certificate;
import <vector>;
import <string>;
import <win32.hpp>;
import boring32.error;
import :functions;

export namespace Boring32::Crypto
{
	class Certificate
	{
		public:
			enum class CertTimeValidity : long
			{
				NotYetValid = -1,
				Valid = 0,
				Expired = 1
			};

		public:
			virtual ~Certificate() = default;
			Certificate() = default;
			Certificate(const Certificate& other)
			{
				Copy(other);
			}

			Certificate(Certificate&& other) noexcept
			{
				Move(other);
			}

			Certificate(PCCERT_CONTEXT certContext, const bool ownedExclusively)
			{
				if (!certContext)
					return;
				m_certContext = ownedExclusively
					? certContext
					: CertDuplicateCertificateContext(certContext);
			}

		public:
			virtual Certificate& operator=(const Certificate& other)
			{
				return Copy(other);
			}

			virtual Certificate& operator=(Certificate&& other) noexcept
			{
				return Move(other);
			}

			virtual bool operator==(PCCERT_CONTEXT const other) const noexcept
			{
				return m_certContext == other;
			}

			virtual operator bool() const noexcept
			{
				return m_certContext != nullptr;
			}

			virtual operator PCCERT_CONTEXT() const noexcept
			{
				return m_certContext;
			}

		public:
			virtual void Close() noexcept
			{
				if (m_certContext)
				{
					CertFreeCertificateContext(m_certContext);
					m_certContext = nullptr;
				}
			}

			virtual void Attach(PCCERT_CONTEXT attachTo)
			{
				Close();
				m_certContext = attachTo;
			}

			virtual void IncreaseRefCount() const noexcept
			{
				if (m_certContext)
				{
					CertDuplicateCertificateContext(m_certContext);
				}
			}

			virtual [[nodiscard]] PCCERT_CONTEXT GetCert() const noexcept
			{
				return m_certContext;
			}

			virtual [[nodiscard]] std::wstring GetFormattedSubject(const DWORD format) const
			{
				return FormatAsnNameBlob(m_certContext->pCertInfo->Subject, format);
			}

			virtual [[nodiscard]] std::wstring GetFormattedIssuer(const DWORD format) const
			{
				return FormatAsnNameBlob(m_certContext->pCertInfo->Issuer, format);
			}

			virtual [[nodiscard]] std::vector<std::byte> GetIssuer() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				if (!m_certContext->pCertInfo)
					return {};
				CERT_NAME_BLOB* blob = &m_certContext->pCertInfo->Issuer;
				return {
					reinterpret_cast<std::byte*>(blob->pbData),
					reinterpret_cast<std::byte*>(blob->pbData + blob->cbData)
				};
			}

			virtual [[nodiscard]] std::vector<std::byte> GetSubject() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				if (!m_certContext->pCertInfo)
					return {};
				CERT_NAME_BLOB* blob = &m_certContext->pCertInfo->Subject;
				return {
					reinterpret_cast<std::byte*>(blob->pbData),
					reinterpret_cast<std::byte*>(blob->pbData + blob->cbData)
				};
			}

			virtual [[nodiscard]] std::wstring GetSignature() const
			{
				return ToBase64WString(InternalCertGetProperty(CERT_SIGNATURE_HASH_PROP_ID));
			}

			virtual [[nodiscard]] std::wstring GetSignatureHashCngAlgorithm() const
			{
				std::vector<std::byte> bytes = InternalCertGetProperty(CERT_SIGN_HASH_CNG_ALG_PROP_ID);
				std::wstring result(
					reinterpret_cast<wchar_t*>(&bytes[0]),
					bytes.size() / sizeof(wchar_t)
				);
				if (!result.empty())
					result.pop_back(); // remove added terminal null
				return result;
			}

			virtual [[nodiscard]] PCCERT_CONTEXT Detach() noexcept
			{
				PCCERT_CONTEXT temp = m_certContext;
				m_certContext = nullptr;
				return temp;
			}

			virtual [[nodiscard]] bool IsValidForCurrentDate() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				return GetTimeValidity(nullptr) == Certificate::CertTimeValidity::Valid;
			}

			virtual [[nodiscard]] CertTimeValidity GetTimeValidity(FILETIME* const ft) const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifytimevalidity
				const long value = CertVerifyTimeValidity(
					ft,
					m_certContext->pCertInfo
				);
				return static_cast<Certificate::CertTimeValidity>(value);
			}

			virtual [[nodiscard]] const FILETIME& GetNotBefore() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				return m_certContext->pCertInfo->NotBefore;
			}

			virtual [[nodiscard]] const FILETIME& GetNotAfter() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				return m_certContext->pCertInfo->NotAfter;
			}

			virtual [[nodiscard]] DWORD GetPublicKeyBitLength() const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");
				// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetpublickeylength
				const DWORD length = CertGetPublicKeyLength(
					X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
					&m_certContext->pCertInfo->SubjectPublicKeyInfo
				);
				if (length == 0)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("CertGetPublicKeyLength() failed", lastError);
				}
				return length;
			}

			virtual [[nodiscard]] PCCERT_CONTEXT Duplicate() const noexcept
			{
				return m_certContext
					? CertDuplicateCertificateContext(m_certContext)
					: nullptr;
			}

		protected:
			virtual Certificate& Copy(const Certificate& other)
			{
				if (this == &other)
					return *this;
				Close();
				if (other.m_certContext)
					m_certContext = CertDuplicateCertificateContext(other.m_certContext);
				return *this;
			}

			virtual Certificate& Move(Certificate& other) noexcept
			{
				Close();
				m_certContext = other.m_certContext;
				other.m_certContext = nullptr;
				return *this;
			}

			virtual std::vector<std::byte> InternalCertGetProperty(
				const DWORD property
			) const
			{
				if (!m_certContext)
					throw Error::Boring32Error("m_certContext is nullptr");

				DWORD sizeInBytes = 0;
				bool succeeded = CertGetCertificateContextProperty(
					m_certContext,
					property,
					nullptr,
					&sizeInBytes
				);
				if (!succeeded)
					throw Error::Win32Error(
						"CertGetCertificateContextProperty() failed (1)",
						GetLastError()
					);

				std::vector<std::byte> returnValue(sizeInBytes);
				succeeded = CertGetCertificateContextProperty(
					m_certContext,
					property,
					&returnValue[0],
					&sizeInBytes
				);
				if (!succeeded)
					throw Error::Win32Error(
						"CertGetCertificateContextProperty() failed (2)",
						GetLastError()
					);
				returnValue.resize(sizeInBytes);

				return returnValue;
			}

		protected:
			// See https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/ns-wincrypt-cert_context
			PCCERT_CONTEXT m_certContext = nullptr;

			/*
			std::wstring Certificate::_GetSubjectName() const
			{
				DWORD type = CERT_X500_NAME_STR;
				DWORD characters = CertGetNameStringW(
					m_certContext,
					CERT_NAME_SIMPLE_DISPLAY_TYPE,
					0,
					(void*)nullptr,
					nullptr,
					0
				);
				std::wstring name(characters, '\0');
				characters = CertGetNameStringW(
					m_certContext,
					CERT_NAME_SIMPLE_DISPLAY_TYPE,
					0,
					(void*)nullptr,
					&name[0],
					characters
				);
				if (characters == 1)
					return L"";
				name.pop_back();
				return name;
			}
			*/
	};
}

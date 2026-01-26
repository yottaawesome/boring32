export module boring32:crypto.certificate;
import std;
import boring32.win32;
import :error;
import :crypto.functions;

export namespace Boring32::Crypto
{
	struct Certificate final
	{
		enum class CertTimeValidity : long
		{
			NotYetValid = -1,
			Valid = 0,
			Expired = 1
		};

		Certificate() = default;
		Certificate(const Certificate& other)
		{
			Copy(other);
		}

		Certificate(Certificate&& other) noexcept
		{
			Move(other);
		}

		Certificate(Win32::PCCERT_CONTEXT certContext, const bool ownedExclusively)
		{
			if (not certContext)
				return;
			m_certContext = ownedExclusively
				? certContext
				: Win32::CertDuplicateCertificateContext(certContext);
		}

		auto operator=(const Certificate& other) -> Certificate&
		{
			return Copy(other);
		}

		auto operator=(Certificate&& other) noexcept -> Certificate&
		{
			return Move(other);
		}

		auto operator==(Win32::PCCERT_CONTEXT const other) const noexcept -> bool
		{
			return m_certContext == other;
		}

		operator bool() const noexcept
		{
			return m_certContext != nullptr;
		}

		operator Win32::PCCERT_CONTEXT() const noexcept
		{
			return m_certContext;
		}

		void Close() noexcept
		{
			if (m_certContext)
			{
				Win32::CertFreeCertificateContext(m_certContext);
				m_certContext = nullptr;
			}
		}

		void Attach(PCCERT_CONTEXT attachTo)
		{
			Close();
			m_certContext = attachTo;
		}

		void IncreaseRefCount() const noexcept
		{
			if (m_certContext)
				CertDuplicateCertificateContext(m_certContext);
		}

		[[nodiscard]] 
		auto GetCert() const noexcept -> Win32::PCCERT_CONTEXT
		{
			return m_certContext;
		}

		[[nodiscard]] 
		auto GetFormattedSubject(const Win32::DWORD format) const -> std::wstring
		{
			return FormatAsnNameBlob(m_certContext->pCertInfo->Subject, format);
		}

		[[nodiscard]] 
		auto GetFormattedIssuer(const Win32::DWORD format) const -> std::wstring
		{
			return FormatAsnNameBlob(m_certContext->pCertInfo->Issuer, format);
		}

		[[nodiscard]] 
		auto GetIssuer() const -> std::vector<std::byte>
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			if (not m_certContext->pCertInfo)
				return {};
			Win32::CERT_NAME_BLOB* blob = &m_certContext->pCertInfo->Issuer;
			return {
				reinterpret_cast<std::byte*>(blob->pbData),
				reinterpret_cast<std::byte*>(blob->pbData + blob->cbData)
			};
		}

		[[nodiscard]] 
		auto GetSubject() const -> std::vector<std::byte>
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			if (not m_certContext->pCertInfo)
				return {};
			Win32::CERT_NAME_BLOB* blob = &m_certContext->pCertInfo->Subject;
			return {
				reinterpret_cast<std::byte*>(blob->pbData),
				reinterpret_cast<std::byte*>(blob->pbData + blob->cbData)
			};
		}

		[[nodiscard]] 
		auto GetSignature() const -> std::wstring
		{
			return ToBase64WString(InternalCertGetProperty(Win32::CertSignatureHashPropId));
		}

		[[nodiscard]] 
		auto GetSignatureHashCngAlgorithm() const -> std::wstring
		{
			std::vector<std::byte> bytes = InternalCertGetProperty(Win32::CertSignHasCngAlgPropId);
			std::wstring result(
				reinterpret_cast<wchar_t*>(&bytes[0]),
				bytes.size() / sizeof(wchar_t)
			);
			if (not result.empty())
				result.pop_back(); // remove added terminal null
			return result;
		}

		[[nodiscard]] 
		auto Detach() noexcept -> Win32::PCCERT_CONTEXT
		{
			Win32::PCCERT_CONTEXT temp = m_certContext;
			m_certContext = nullptr;
			return temp;
		}

		[[nodiscard]] 
		auto IsValidForCurrentDate() const -> bool
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			return GetTimeValidity(nullptr) == Certificate::CertTimeValidity::Valid;
		}

		[[nodiscard]] 
		auto GetTimeValidity(Win32::FILETIME* const ft) const -> CertTimeValidity
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");

			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifytimevalidity
			long value = Win32::CertVerifyTimeValidity(ft, m_certContext->pCertInfo);
			return static_cast<Certificate::CertTimeValidity>(value);
		}

		[[nodiscard]] 
		auto GetNotBefore() const -> Win32::FILETIME
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			return m_certContext->pCertInfo->NotBefore;
		}

		[[nodiscard]] 
		auto GetNotAfter() const -> const Win32::FILETIME&
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			return m_certContext->pCertInfo->NotAfter;
		}

		[[nodiscard]] 
		auto GetPublicKeyBitLength() const -> Win32::DWORD
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");
			// https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetpublickeylength
			Win32::DWORD length = Win32::CertGetPublicKeyLength(
				Win32::X509AsnEncoding | Win32::Pkcs7AsnEncoding,
				&m_certContext->pCertInfo->SubjectPublicKeyInfo
			);
			if (length == 0)
				throw Error::Win32Error(Win32::GetLastError(), "CertGetPublicKeyLength() failed");
			return length;
		}

		[[nodiscard]] 
		auto Duplicate() const noexcept -> Win32::PCCERT_CONTEXT
		{
			return m_certContext
				? Win32::CertDuplicateCertificateContext(m_certContext)
				: nullptr;
		}

		[[nodiscard]] 
		auto GetHash() const -> std::wstring
		{
			return m_certContext 
				? ToBase64WString(InternalCertGetProperty(Win32::_CERT_HASH_PROP_ID))
				: L"";
		}

		private:
		auto Copy(const Certificate& other) -> Certificate&
		{
			if (this == &other)
				return *this;
			Close();
			if (other.m_certContext)
				m_certContext = Win32::CertDuplicateCertificateContext(other.m_certContext);
			return *this;
		}

		auto Move(Certificate& other) noexcept -> Certificate&
		{
			Close();
			m_certContext = other.m_certContext;
			other.m_certContext = nullptr;
			return *this;
		}

		auto InternalCertGetProperty(Win32::DWORD property) const -> std::vector<std::byte>
		{
			if (not m_certContext)
				throw Error::Boring32Error("m_certContext is nullptr");

			Win32::DWORD sizeInBytes = 0;
			bool succeeded = Win32::CertGetCertificateContextProperty(
				m_certContext,
				property,
				nullptr,
				&sizeInBytes
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "CertGetCertificateContextProperty() failed (1)");

			std::vector<std::byte> returnValue(sizeInBytes);
			succeeded = Win32::CertGetCertificateContextProperty(
				m_certContext,
				property,
				&returnValue[0],
				&sizeInBytes
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "CertGetCertificateContextProperty() failed (2)");
			returnValue.resize(sizeInBytes);

			return returnValue;
		}

		// See https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/ns-wincrypt-cert_context
		Win32::PCCERT_CONTEXT m_certContext = nullptr;

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

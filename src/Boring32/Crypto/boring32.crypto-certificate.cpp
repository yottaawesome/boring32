module;

#include <vector>
#include <string>
#include <source_location>

module boring32.crypto:certificate;
import boring32.error;
import :functions;

namespace Boring32::Crypto
{
	Certificate::~Certificate()
	{
		Close();
	}

	Certificate::Certificate(PCCERT_CONTEXT certContext, const bool ownedExclusively)
	:	m_certContext(nullptr)
	{
		if (!certContext)
			return; 
		m_certContext = ownedExclusively
			? certContext 
			: CertDuplicateCertificateContext(certContext);
	}

	Certificate::Certificate(const Certificate& other)
	:	m_certContext(nullptr)
	{
		Copy(other);
	}

	Certificate& Certificate::operator=(const Certificate& other)
	{
		return Copy(other);
	}

	Certificate& Certificate::Copy(const Certificate& other)
	{
		Close();
		if (other.m_certContext)
			m_certContext = CertDuplicateCertificateContext(other.m_certContext);
		return *this;
	}

	Certificate::Certificate(Certificate&& other) noexcept
	:	m_certContext(nullptr)
	{
		Move(other);
	}

	Certificate& Certificate::operator=(Certificate&& other) noexcept
	{
		return Move(other);
	}

	bool Certificate::operator==(PCCERT_CONTEXT const other) const noexcept
	{
		return m_certContext == other;
	}

	Certificate::operator bool() const noexcept
	{
		return m_certContext != nullptr;
	}

	Certificate::operator PCCERT_CONTEXT() const noexcept
	{
		return m_certContext;
	}

	Certificate& Certificate::Move(Certificate& other) noexcept
	{
		Close();
		m_certContext = other.m_certContext;
		other.m_certContext = nullptr;
		return *this;
	}

	void Certificate::Close() noexcept
	{
		if (m_certContext)
		{
			CertFreeCertificateContext(m_certContext);
			m_certContext = nullptr;
		}
	}

	PCCERT_CONTEXT Certificate::GetCert() const noexcept
	{
		return m_certContext;
	}

	std::wstring Certificate::GetFormattedSubject(const DWORD format) const
	{
		return FormatAsnNameBlob(m_certContext->pCertInfo->Subject, format);
	}

	std::wstring Certificate::GetFormattedIssuer(const DWORD format) const
	{
		return FormatAsnNameBlob(m_certContext->pCertInfo->Issuer, format);
	}

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

	std::vector<std::byte> Certificate::GetIssuer() const
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

	std::vector<std::byte> Certificate::GetSubject() const
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

	std::wstring Certificate::GetSignature() const
	{
		return ToBase64WString(InternalCertGetProperty(CERT_SIGNATURE_HASH_PROP_ID));
	}
	
	std::wstring Certificate::GetSignatureHashCngAlgorithm() const
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

	void Certificate::Attach(PCCERT_CONTEXT attachTo)
	{
		Close();
		m_certContext = attachTo;
	}

	PCCERT_CONTEXT Certificate::Detach() noexcept
	{
		PCCERT_CONTEXT temp = m_certContext;
		m_certContext = nullptr;
		return temp;
	}

	std::vector<std::byte> Certificate::InternalCertGetProperty(const DWORD property) const
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

	bool Certificate::IsValidForCurrentDate() const
	{
		if (!m_certContext)
			throw Error::Boring32Error("m_certContext is nullptr");
		return GetTimeValidity(nullptr) == Certificate::CertTimeValidity::Valid;
	}

	Certificate::CertTimeValidity Certificate::GetTimeValidity(FILETIME* const ft) const
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

	const FILETIME& Certificate::GetNotBefore() const
	{
		if (!m_certContext)
			throw Error::Boring32Error("m_certContext is nullptr");
		return m_certContext->pCertInfo->NotBefore;
	}

	const FILETIME& Certificate::GetNotAfter() const
	{
		if (!m_certContext)
			throw Error::Boring32Error("m_certContext is nullptr");
		return m_certContext->pCertInfo->NotAfter;
	}
}

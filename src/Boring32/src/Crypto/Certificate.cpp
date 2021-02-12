#include "pch.hpp"
#include "vector"
#include "include/Error/Error.hpp"
#include "include/Crypto/CryptoFuncs.hpp"
#include "include/Crypto/Certificate.hpp"

namespace Boring32::Crypto
{
	Certificate::~Certificate()
	{
		Close();
	}

	Certificate::Certificate()
	:	m_certContext(nullptr)
	{ }

	Certificate::Certificate(PCCERT_CONTEXT certContext)
	:	m_certContext(certContext)
	{ }

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
		return InternalGetFormattedName(m_certContext->pCertInfo->Subject, format);
	}

	std::wstring Certificate::GetFormattedIssuer(const DWORD format) const
	{
		return InternalGetFormattedName(m_certContext->pCertInfo->Issuer, format);
	}

	std::wstring Certificate::InternalGetFormattedName(
		CERT_NAME_BLOB& certName,
		const DWORD format
	) const
	{
		if (m_certContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_certContext is null");
		
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certnametostrw
		DWORD characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			&certName,
			format,
			nullptr,
			0
		);
		if (characterSize == 0)
			return L"";

		std::wstring name(characterSize, '\0');
		characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			&certName,
			format,
			&name[0],
			(DWORD)name.size()
		);
		name.pop_back(); // remove excess null character

		return name;
	}

	std::wstring Certificate::GetSignature() const
	{
		std::vector<std::byte> bytes = InternalCertGetProperty(CERT_SIGNATURE_HASH_PROP_ID);
		return ToBase64WString(bytes);
	}
	
	std::wstring Certificate::GetSignatureHashCngAlgorithm() const
	{
		std::vector<std::byte> bytes = InternalCertGetProperty(CERT_SIGN_HASH_CNG_ALG_PROP_ID);
		return std::wstring((wchar_t*)&bytes[0], bytes.size() / sizeof(wchar_t));
	}

	void Certificate::Attach(PCCERT_CONTEXT const attachTo)
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
		if (m_certContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_certContext is nullptr");

		DWORD sizeInBytes = 0;
		bool succeeded = CertGetCertificateContextProperty(
			m_certContext,
			property,
			nullptr,
			&sizeInBytes
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CertGetCertificateContextProperty() failed (1)",
				GetLastError()
			);

		std::vector<std::byte> returnValue(sizeInBytes);
		succeeded = CertGetCertificateContextProperty(
			m_certContext,
			property,
			&returnValue[0],
			&sizeInBytes
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CertGetCertificateContextProperty() failed (2)",
				GetLastError()
			);
		returnValue.resize(sizeInBytes);
		return returnValue;
	}
}

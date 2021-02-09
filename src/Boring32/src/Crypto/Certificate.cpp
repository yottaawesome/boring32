#include "pch.hpp"
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

	std::wstring Certificate::GetFormattedSubjectName(const DWORD format) const
	{
		if (m_certContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_certContext is null");
			
		DWORD characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			&m_certContext->pCertInfo->Subject,
			format,
			nullptr,
			0
		);
		if (characterSize == 0)
			return L"";

		std::wstring name(characterSize, '\0');
		characterSize = CertNameToStrW(
			X509_ASN_ENCODING,
			&m_certContext->pCertInfo->Subject,
			format,
			&name[0],
			(DWORD)name.size()
		);
		name.pop_back(); // remove excess null character
		return name;
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
}

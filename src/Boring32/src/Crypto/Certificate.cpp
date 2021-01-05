#include "pch.hpp"
#include "include/Crypto/Certificate.hpp"

namespace Boring32::Crypto
{
	Certificate::~Certificate()
	{
		Close();
	}

	Certificate::Certificate()
		: m_certContext(nullptr)
	{ }

	Certificate::Certificate(PCCERT_CONTEXT certContext)
		: m_certContext(certContext)
	{ }

	Certificate::Certificate(const Certificate& other)
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
	{
		Move(other);
	}

	Certificate& Certificate::operator=(Certificate&& other) noexcept
	{
		return Move(other);
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
}

#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Crypto/CertificateChain.hpp"

namespace Boring32::Crypto
{
	CertificateChain::~CertificateChain()
	{
		Close();
	}

	CertificateChain::CertificateChain()
	:	m_chainContext(nullptr)
	{ }

	CertificateChain::CertificateChain(const CertificateChain& other)
	:	m_chainContext(nullptr)
	{
		Copy(other);
	}

	CertificateChain::CertificateChain(CertificateChain&& other) noexcept
	:	m_chainContext(nullptr)
	{
		Move(other);
	}
	
	CertificateChain::CertificateChain(
		PCCERT_CHAIN_CONTEXT chainContext,
		const bool makeCopy
	)
	:	m_chainContext(chainContext)
	{
		if (makeCopy)
			m_chainContext = CertDuplicateCertificateChain(chainContext);
	}

	CertificateChain::CertificateChain(const Certificate& contextToBuildFrom)
	:	m_chainContext(nullptr)
	{
		GenerateFrom(
			(CERT_CONTEXT*)contextToBuildFrom.GetCert(), 
			nullptr
		);
	}

	CertificateChain::CertificateChain(
		const Certificate& contextToBuildFrom,
		const CertStore& store
	)
	:	m_chainContext(nullptr)
	{
		GenerateFrom(
			(CERT_CONTEXT*)contextToBuildFrom.GetCert(), 
			store.GetHandle()
		);
	}

	CertificateChain& CertificateChain::operator=(const CertificateChain& other)
	{
		return Copy(other);
	}

	CertificateChain& CertificateChain::operator=(CertificateChain&& other) noexcept
	{
		return Move(other);
	}

	void CertificateChain::Close() noexcept
	{
		if (m_chainContext)
		{
			CertFreeCertificateChain(m_chainContext);
			m_chainContext = nullptr;
		}
	}
	
	void CertificateChain::Verify()
	{
		CERT_CHAIN_POLICY_PARA para{
			.cbSize = sizeof(para),
			.dwFlags = 0,
			.pvExtraPolicyPara = nullptr
		};
		CERT_CHAIN_POLICY_STATUS status{
			.cbSize = sizeof(status)
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifycertificatechainpolicy
		bool succeeded = CertVerifyCertificateChainPolicy(
			CERT_CHAIN_POLICY_SSL,
			m_chainContext,
			&para,
			&status
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CertVerifyCertificateChainPolicy() failed");
	}
	
	PCCERT_CHAIN_CONTEXT CertificateChain::GetChain() const noexcept
	{
		return m_chainContext;
	}

	CertificateChain& CertificateChain::Copy(const CertificateChain& other)
	{
		if (&other == this)
			return *this;
		
		Close();
		m_chainContext = CertDuplicateCertificateChain(other.m_chainContext);

		return *this;
	}

	CertificateChain& CertificateChain::Move(CertificateChain& other) noexcept
	{
		Close();
		m_chainContext = other.m_chainContext;
		other.m_chainContext = nullptr;

		return *this;
	}

	void CertificateChain::GenerateFrom(
		PCCERT_CONTEXT contextToBuildFrom,
		HCERTSTORE store
	)
	{
		CERT_ENHKEY_USAGE        EnhkeyUsage;
		CERT_USAGE_MATCH         CertUsage;
		EnhkeyUsage.cUsageIdentifier = 0;
		EnhkeyUsage.rgpszUsageIdentifier = nullptr;
		CertUsage.dwType = USAGE_MATCH_TYPE_AND;
		CertUsage.Usage = EnhkeyUsage;
		CERT_CHAIN_PARA certChainParams{
			.cbSize = sizeof(certChainParams),
			.RequestedUsage = CertUsage
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certgetcertificatechain
		// https://docs.microsoft.com/en-us/windows/win32/seccrypto/example-c-program-creating-a-certificate-chain
		bool succeeded = CertGetCertificateChain(
			nullptr,
			(PCCERT_CONTEXT)contextToBuildFrom,
			nullptr,
			store,
			&certChainParams,
			CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
			nullptr,
			(PCCERT_CHAIN_CONTEXT*)&m_chainContext
		);
		if (succeeded == false)
			throw Error::Win32Error(
				__FUNCSIG__ ": CertGetCertificateChain() failed", 
				GetLastError()
			);
	}
}
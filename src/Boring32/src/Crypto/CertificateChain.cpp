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
	{
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

	void CertificateChain::Close()
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
		bool succeeded = CertVerifyCertificateChainPolicy(
			CERT_CHAIN_POLICY_SSL,
			(PCCERT_CHAIN_CONTEXT)m_chainContext,
			&para,
			&status
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": CertVerifyCertificateChainPolicy() failed");
	}

	void CertificateChain::GenerateFrom(
		CERT_CONTEXT* contextToBuildFrom,
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
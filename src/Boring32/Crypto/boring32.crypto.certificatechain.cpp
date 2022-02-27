module;

#include <vector>
#include <string>
#include <format>
#include <stdexcept>
#include <source_location>
#include <Windows.h>
#include <wincrypt.h>

module boring32.crypto.certificatechain;
import boring32.error.win32error;

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
		const bool takeExclusiveOwnership
	)
	:	m_chainContext(nullptr)
	{
		m_chainContext = takeExclusiveOwnership
			? chainContext
			: CertDuplicateCertificateChain(chainContext);
	}

	CertificateChain::CertificateChain(const Certificate& contextToBuildFrom)
	:	m_chainContext(nullptr)
	{
		GenerateFrom(
			contextToBuildFrom.GetCert(), 
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
			contextToBuildFrom.GetCert(), 
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
		// TODO: need to verify this actually works
		if (m_chainContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");

		CERT_CHAIN_POLICY_PARA para{
			.cbSize = sizeof(para),
			.dwFlags = 0,
			.pvExtraPolicyPara = nullptr
		};
		CERT_CHAIN_POLICY_STATUS status{
			.cbSize = sizeof(status)
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifycertificatechainpolicy
		const bool succeeded = CertVerifyCertificateChainPolicy(
			CERT_CHAIN_POLICY_SSL,
			m_chainContext,
			&para,
			&status
		);
		if (succeeded == false)
			throw Error::Win32Error(std::source_location::current(), "CertVerifyCertificateChainPolicy() failed");
	}
	
	PCCERT_CHAIN_CONTEXT CertificateChain::GetChainContext() const noexcept
	{
		return m_chainContext;
	}

	std::vector<Certificate> CertificateChain::GetCertChainAt(
		const DWORD chainIndex
	) const
	{
		if (m_chainContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");
		
		if (chainIndex >= m_chainContext->cChain)
			throw std::invalid_argument(
				std::format(
					"{}: expected index to be less than {} but got an index of {}",
					__FUNCSIG__,
					m_chainContext->cChain,
					chainIndex
				));

		std::vector<Certificate> certsInChain;
		CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
		// This probably should never happen, but guard just in case
		if (simpleChain == nullptr)
			throw std::runtime_error(
				std::format(
					"{}: the simpleChain at {} was unexpectedly null",
					__FUNCSIG__,
					chainIndex
				));

		for (
			DWORD certIndexInChain = 0;
			certIndexInChain < simpleChain->cElement;
			certIndexInChain++
		)
			certsInChain.emplace_back(
				simpleChain->rgpElement[certIndexInChain]->pCertContext,
				false
			);

		return certsInChain;
	}

	Certificate CertificateChain::GetCertAt(
		const DWORD chainIndex,
		const DWORD certIndex
	) const
	{
		if (m_chainContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");
		if (chainIndex >= m_chainContext->cChain)
			throw std::invalid_argument(
				std::format(
					"{}: expected chainIndex to be less than {} but got an index of {}",
					__FUNCSIG__,
					m_chainContext->cChain,
					chainIndex
				)
			);

		CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
		if (simpleChain == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": simpleChain is null");
		if (certIndex >= simpleChain->cElement)
			throw std::invalid_argument(
				std::format(
					"{}: expected certIndex to be less than {} but got an index of {}"
					__FUNCSIG__,
					simpleChain->cElement,
					certIndex
				)
			);
		
		return { simpleChain->rgpElement[certIndex]->pCertContext, false };
	}

	Certificate CertificateChain::GetFirstCertAt(
		const DWORD chainIndex
	) const
	{
		if (m_chainContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");
		if (chainIndex >= m_chainContext->cChain) 
			throw std::invalid_argument(
				std::format(
					"{}: expected index to be less than {} but got an index of {}",
					__FUNCSIG__,
					m_chainContext->cChain,
					chainIndex
			));

		CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
		if (simpleChain == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": simpleChain is null");
		if (simpleChain->cElement == 0)
			return {};
		return { simpleChain->rgpElement[0]->pCertContext, false };
	}

	Certificate CertificateChain::GetLastCertAt(
		const DWORD chainIndex
	) const
	{
		if (m_chainContext == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");
		if (chainIndex >= m_chainContext->cChain)
			throw std::invalid_argument(
				std::format(
					"{}: expected index to be less than {} but got an index of {}",
					__FUNCSIG__,
					m_chainContext->cChain,
					chainIndex
				));

		CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
		if (simpleChain == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": simpleChain is null");
		if (simpleChain->cElement == 0)
			return {};
		return { simpleChain->rgpElement[simpleChain->cElement - 1]->pCertContext, false };
	}

	CertStore CertificateChain::ChainToStore(const DWORD chainIndex) const
	{
		std::vector<Certificate> certificatesInChain = GetCertChainAt(chainIndex);
		CertStore temporaryStore(L"", CertStoreType::InMemory);
		for (const Certificate& cert : certificatesInChain)
			temporaryStore.ImportCert(cert.GetCert());
		
		return temporaryStore;
	}

	CertificateChain& CertificateChain::Copy(const CertificateChain& other)
	{
		if (&other == this)
			return *this;
		
		Close();
		if (other.m_chainContext)
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
		if (contextToBuildFrom == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_chainContext is null");

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
		const bool succeeded = CertGetCertificateChain(
			nullptr,
			contextToBuildFrom,
			nullptr,
			store,
			&certChainParams,
			CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
			nullptr,
			&m_chainContext
		);
		if (succeeded == false)
			throw Error::Win32Error(
				std::source_location::current(),
				"CertGetCertificateChain() failed", 
				GetLastError()
			);
	}
}
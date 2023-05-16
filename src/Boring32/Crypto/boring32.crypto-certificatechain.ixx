export module boring32.crypto:certificatechain;
import :certificate;
import <vector>;
import <win32.hpp>;
import :functions;
import <string>;
import <format>;
import boring32.error;

export namespace Boring32::Crypto
{
	enum ChainVerificationPolicy : std::uintptr_t
	{
		Base = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_BASE),
		Authenticode = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_AUTHENTICODE),
		AuthenticodeTS = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_AUTHENTICODE_TS),
		SSL = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_SSL),
		BasicConstraints = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_BASIC_CONSTRAINTS),
		NTAuth = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_NT_AUTH),
		MicrosoftRoot = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_MICROSOFT_ROOT),
		EV = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_EV),
		SSLF12 = reinterpret_cast<std::uintptr_t>(CERT_CHAIN_POLICY_SSL_F12)
	};

	class CertificateChain final
	{
		public:
			~CertificateChain()
			{
				Close();
			}

			CertificateChain() = default;

			CertificateChain(const CertificateChain& other)
			{
				Copy(other);
			}

			CertificateChain(CertificateChain&& other) noexcept
			{
				Move(other);
			}

			CertificateChain& operator=(const CertificateChain& other)
			{
				Close();
				return Copy(other);
			}

			CertificateChain& operator=(CertificateChain&& other) noexcept
			{
				Close();
				return Move(other);
			}

		public:
			CertificateChain(
				PCCERT_CHAIN_CONTEXT chainContext,
				const bool takeExclusiveOwnership
			)
			{
				m_chainContext = takeExclusiveOwnership
					? chainContext
					: CertDuplicateCertificateChain(chainContext);
			}

			CertificateChain(
				const Certificate& contextToBuildFrom
			)
			{
				m_chainContext = GenerateChainFrom(
					contextToBuildFrom.GetCert(),
					nullptr
				);
			}

			CertificateChain(
				const Certificate& contextToBuildFrom, 
				HCERTSTORE store
			)
			{
				m_chainContext = GenerateChainFrom(
					contextToBuildFrom.GetCert(),
					store
				);
			}

		public:
			void Close() noexcept
			{
				if (m_chainContext)
				{
					CertFreeCertificateChain(m_chainContext);
					m_chainContext = nullptr;
				}
			}

			bool Verify(const ChainVerificationPolicy policy)
			{
				// TODO: need to verify this actually works
				if (!m_chainContext)
					throw Error::Boring32Error("m_chainContext is null");

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
					reinterpret_cast<PCSTR>(policy),
					m_chainContext,
					&para,
					&status
				);
				if (!succeeded)
					throw Error::Win32Error("CertVerifyCertificateChainPolicy() failed");
				return status.dwError == 0;
			}

			PCCERT_CHAIN_CONTEXT GetChainContext() const noexcept
			{
				return m_chainContext;
			}

			std::vector<Certificate> GetCertChainAt(const DWORD chainIndex) const
			{
				if (!m_chainContext)
					throw Error::Boring32Error("m_chainContext is null");

				if (chainIndex >= m_chainContext->cChain)
					throw Error::Boring32Error(
						"Expected index to be less than {} but got an index of {}",
						std::source_location::current(),
						std::stacktrace::current(),
						m_chainContext->cChain,
						chainIndex
					);

				std::vector<Certificate> certsInChain;
				CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
				// This probably should never happen, but guard just in case
				if (!simpleChain) 
					throw Error::Boring32Error(
						"The simpleChain at {} was unexpectedly null",
						std::source_location::current(),
						std::stacktrace::current(),
						chainIndex
					);

				for (
					DWORD certIndexInChain = 0;
					certIndexInChain < simpleChain->cElement;
					certIndexInChain++
				)
				{
					certsInChain.emplace_back(
						simpleChain->rgpElement[certIndexInChain]->pCertContext,
						false
					);
				}

				return certsInChain;
			}

			Certificate GetCertAt(
				const DWORD chainIndex, 
				const DWORD certIndex
			) const
			{
				if (!m_chainContext)
					throw Error::Boring32Error("m_chainContext is null");
				if (chainIndex >= m_chainContext->cChain)
					throw Error::Boring32Error(
						"Expected chainIndex to be less than {} but got an index of {}",
						std::source_location::current(),
						std::stacktrace::current(),
						m_chainContext->cChain,
						chainIndex
					);

				CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
				if (!simpleChain)
					throw Error::Boring32Error("simpleChain is null");
				if (certIndex >= simpleChain->cElement)
					throw Error::Boring32Error(
						"Expected certIndex to be less than {} but got an index of {}",
						std::source_location::current(),
						std::stacktrace::current(),
						simpleChain->cElement,
						certIndex
					);

				return { simpleChain->rgpElement[certIndex]->pCertContext, false };
			}

			Certificate GetFirstCertAt(
				const DWORD chainIndex
			) const
			{
				if (!m_chainContext)
					throw Error::Boring32Error("m_chainContext is null");
				if (chainIndex >= m_chainContext->cChain)
					throw Error::Boring32Error(
						std::format(
							"Expected index to be less than {} but got an index of {}",
							m_chainContext->cChain,
							chainIndex
						));

				CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
				if (!simpleChain)
					throw Error::Boring32Error("simpleChain is null");
				if (simpleChain->cElement == 0)
					return {};
				return { simpleChain->rgpElement[0]->pCertContext, false };
			}

			Certificate GetLastCertAt(
				const DWORD chainIndex
			) const
			{
				if (!m_chainContext)
					throw Error::Boring32Error("m_chainContext is null");
				if (chainIndex >= m_chainContext->cChain)
					throw Error::Boring32Error(
						"Expected index to be less than {} but got an index of {}",
						std::source_location::current(),
						std::stacktrace::current(),
						m_chainContext->cChain,
						chainIndex
					);

				CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
				if (!simpleChain)
					throw Error::Boring32Error("simpleChain is null");
				if (simpleChain->cElement == 0)
					return {};
				return { simpleChain->rgpElement[simpleChain->cElement - 1]->pCertContext, false };
			}

			/*CertStore ChainToStore(const DWORD chainIndex) const
			{
				std::vector<Certificate> certificatesInChain = GetCertChainAt(chainIndex);
				CertStore temporaryStore(L"", CertStoreType::InMemory);
				for (const Certificate& cert : certificatesInChain)
					temporaryStore.ImportCert(cert.GetCert());

				return temporaryStore;
			}*/

		private:
			CertificateChain& Copy(const CertificateChain& other)
			{
				if (&other == this)
					return *this;

				Close();
				if (other.m_chainContext)
					m_chainContext = CertDuplicateCertificateChain(other.m_chainContext);

				return *this;
			}

			CertificateChain& Move(CertificateChain& other) noexcept
			{
				Close();
				m_chainContext = other.m_chainContext;
				other.m_chainContext = nullptr;

				return *this;
			}

		private:
			PCCERT_CHAIN_CONTEXT m_chainContext = nullptr;
	};
}
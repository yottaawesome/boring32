export module boring32:crypto_certificatechain;
import std;
import boring32.win32;
import :error;
import :crypto_certificate;
import :crypto_functions;

export namespace Boring32::Crypto
{
	using ChainVerificationPolicy = Win32::ChainVerificationPolicy;

	struct CertificateChain final
	{
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

		CertificateChain(Win32::PCCERT_CHAIN_CONTEXT chainContext, const bool takeExclusiveOwnership)
		{
			m_chainContext = takeExclusiveOwnership
				? chainContext
				: Win32::CertDuplicateCertificateChain(chainContext);
		}

		CertificateChain(const Certificate& contextToBuildFrom)
		{
			m_chainContext = GenerateChainFrom(contextToBuildFrom.GetCert(), nullptr);
		}

		CertificateChain(const Certificate& contextToBuildFrom, Win32::HCERTSTORE store)
		{
			m_chainContext = GenerateChainFrom(
				contextToBuildFrom.GetCert(),
				store
			);
		}

		void Close() noexcept
		{
			if (m_chainContext)
			{
				Win32::CertFreeCertificateChain(m_chainContext);
				m_chainContext = nullptr;
			}
		}

		bool Verify(const ChainVerificationPolicy policy)
		{
			// TODO: need to verify this actually works
			if (!m_chainContext)
				throw Error::Boring32Error("m_chainContext is null");

			Win32::CERT_CHAIN_POLICY_PARA para{
				.cbSize = sizeof(para),
				.dwFlags = 0,
				.pvExtraPolicyPara = nullptr
			};
			Win32::CERT_CHAIN_POLICY_STATUS status{
				.cbSize = sizeof(status)
			};
			// https://docs.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-certverifycertificatechainpolicy
			bool succeeded = Win32::CertVerifyCertificateChainPolicy(
				reinterpret_cast<Win32::PCSTR>(policy),
				m_chainContext,
				&para,
				&status
			);
			if (not succeeded)
				throw Error::Win32Error("CertVerifyCertificateChainPolicy() failed");
			return status.dwError == 0;
		}

		Win32::PCCERT_CHAIN_CONTEXT GetChainContext() const noexcept
		{
			return m_chainContext;
		}

		std::vector<Certificate> GetCertChainAt(const Win32::DWORD chainIndex) const
		{
			if (not m_chainContext)
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
			Win32::CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
			// This probably should never happen, but guard just in case
			if (not simpleChain) throw Error::Boring32Error(
				"The simpleChain at {} was unexpectedly null",
				std::source_location::current(),
				std::stacktrace::current(),
				chainIndex
			);

			for (Win32::DWORD certIndexInChain = 0; certIndexInChain < simpleChain->cElement; certIndexInChain++)
				certsInChain.emplace_back(simpleChain->rgpElement[certIndexInChain]->pCertContext, false);

			return certsInChain;
		}

		Certificate GetCertAt(Win32::DWORD chainIndex, Win32::DWORD certIndex) const
		{
			if (not m_chainContext)
				throw Error::Boring32Error("m_chainContext is null");
			if (chainIndex >= m_chainContext->cChain)
				throw Error::Boring32Error(
					"Expected chainIndex to be less than {} but got an index of {}",
					std::source_location::current(),
					std::stacktrace::current(),
					m_chainContext->cChain,
					chainIndex
				);

			Win32::CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
			if (not simpleChain)
				throw Error::Boring32Error("simpleChain is null");
			if (certIndex >= simpleChain->cElement) throw Error::Boring32Error(
				"Expected certIndex to be less than {} but got an index of {}",
				std::source_location::current(),
				std::stacktrace::current(),
				simpleChain->cElement,
				certIndex
			);

			return { simpleChain->rgpElement[certIndex]->pCertContext, false };
		}

		Certificate GetFirstCertAt(Win32::DWORD chainIndex) const
		{
			if (not m_chainContext)
				throw Error::Boring32Error("m_chainContext is null");
			if (chainIndex >= m_chainContext->cChain)
				throw Error::Boring32Error(
					"Expected index to be less than {} but got an index of {}",
					std::source_location::current(),
					std::stacktrace::current(),
					m_chainContext->cChain,
					chainIndex
				);

			Win32::CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
			if (not simpleChain)
				throw Error::Boring32Error("simpleChain is null");
			if (simpleChain->cElement == 0)
				return {};
			return { simpleChain->rgpElement[0]->pCertContext, false };
		}

		Certificate GetLastCertAt(Win32::DWORD chainIndex) const
		{
			if (not m_chainContext)
				throw Error::Boring32Error("m_chainContext is null");
			if (chainIndex >= m_chainContext->cChain)
				throw Error::Boring32Error(
					"Expected index to be less than {} but got an index of {}",
					std::source_location::current(),
					std::stacktrace::current(),
					m_chainContext->cChain,
					chainIndex
				);

			Win32::CERT_SIMPLE_CHAIN* simpleChain = m_chainContext->rgpChain[chainIndex];
			if (not simpleChain)
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
				m_chainContext = Win32::CertDuplicateCertificateChain(other.m_chainContext);

			return *this;
		}

		CertificateChain& Move(CertificateChain& other) noexcept
		{
			if (&other == this)
				return *this;

			Close();
			m_chainContext = other.m_chainContext;
			other.m_chainContext = nullptr;

			return *this;
		}

		Win32::PCCERT_CHAIN_CONTEXT m_chainContext = nullptr;
	};
}
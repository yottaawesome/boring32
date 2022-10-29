module;

#include <vector>
#include <Windows.h>
#include <wincrypt.h>

export module boring32.crypto:certificatechain;
import :certificate;
import :certstore;

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

	class CertificateChain
	{
		public:
			virtual ~CertificateChain();
			CertificateChain();
			CertificateChain(const CertificateChain& other);
			CertificateChain(CertificateChain&& other) noexcept;

		public:
			CertificateChain(
				PCCERT_CHAIN_CONTEXT m_chainContext, 
				const bool takeExclusiveOwnership
			);
			CertificateChain(
				const Certificate& contextToBuildFrom
			);
			CertificateChain(
				const Certificate& contextToBuildFrom, 
				const CertStore& store
			);

		public:
			virtual CertificateChain& operator=(const CertificateChain& other);
			virtual CertificateChain& operator=(CertificateChain&& other) noexcept;

		public:
			virtual void Close() noexcept;
			virtual bool Verify(const ChainVerificationPolicy policy);
			virtual PCCERT_CHAIN_CONTEXT GetChainContext() const noexcept;
			virtual std::vector<Certificate> GetCertChainAt(const DWORD chainIndex) const;
			virtual Certificate GetCertAt(
				const DWORD chainIndex, 
				const DWORD certIndex
			) const;
			virtual Certificate GetFirstCertAt(
				const DWORD chainIndex
			) const;
			virtual Certificate GetLastCertAt(
				const DWORD chainIndex
			) const;
			virtual CertStore ChainToStore(const DWORD chainIndex) const;

		protected:
			virtual CertificateChain& Copy(const CertificateChain& other);
			virtual CertificateChain& Move(CertificateChain& other) noexcept;
			virtual void GenerateFrom(
				PCCERT_CONTEXT contextToBuildFrom, 
				HCERTSTORE store
			);

		protected:
			PCCERT_CHAIN_CONTEXT m_chainContext;
	};
}
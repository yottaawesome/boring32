#pragma once
#include <Windows.h>
#include <wincrypt.h>
#include "Certificate.hpp"
#include "CertStore.hpp"

namespace Boring32::Crypto
{
	class CertificateChain
	{
		public:
			virtual ~CertificateChain();
			CertificateChain();
			CertificateChain(const CertificateChain& other);
			CertificateChain(CertificateChain&& other) noexcept;
			CertificateChain(
				PCCERT_CHAIN_CONTEXT m_chainContext, 
				const bool makeCopy
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
			virtual void Verify();
			virtual PCCERT_CHAIN_CONTEXT GetChain() const noexcept;

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
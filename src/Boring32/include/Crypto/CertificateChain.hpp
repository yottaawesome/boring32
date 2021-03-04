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
			CertificateChain(
				const Certificate& contextToBuildFrom
			);
			CertificateChain(
				const Certificate& contextToBuildFrom, 
				const CertStore& store
			);

		public:
			virtual void Close();
			virtual void Verify();

		protected:
			virtual void GenerateFrom(
				CERT_CONTEXT* contextToBuildFrom, 
				HCERTSTORE store
			);

		protected:
			CERT_CHAIN_CONTEXT* m_chainContext;
	};
}
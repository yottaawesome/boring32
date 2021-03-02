#pragma once
#include "Certificate.hpp"
#include "CertStore.hpp"

namespace Boring32::Crypto
{
	class TempCertImport
	{
		public:
			virtual ~TempCertImport();
			TempCertImport(CertStore& store, Certificate& toImport);

		protected:
			CertStore m_store;
			std::wstring m_thumbprint;
	};
}

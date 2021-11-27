module;

#include <string>
#include "include/Crypto/Certificate.hpp"
#include "include/Crypto/CertStore.hpp"

export module boring32.crypto.tempcertimport;

export namespace Boring32::Crypto
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

module;

#include <string>

export module boring32.crypto.tempcertimport;
import boring32.crypto.certificate;
import boring32.crypto.certstore;

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
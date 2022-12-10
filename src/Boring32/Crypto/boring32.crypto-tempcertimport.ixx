export module boring32.crypto:tempcertimport;
import :certificate;
import :certstore;
import <string>;
import <win32.hpp>;

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

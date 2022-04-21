module;

#include <string>
#include <Windows.h>
#include <dpapi.h> // not including this header causes symbol has already been defined error
#include <Wincrypt.h>

export module boring32.crypto:tempcertimport;
import :certificate;
import :certstore;

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

export module boring32.crypto:tempcertimport;
import :certificate;
import :certstore;
import <string>;
import <win32.hpp>;
import <iostream>;

export namespace Boring32::Crypto
{
	class TempCertImport
	{
		public:
			virtual ~TempCertImport()
			{
				try
				{
					Certificate certToDelete = m_store.GetCertByByBase64Signature(m_thumbprint);
					m_store.DeleteCert(certToDelete.GetCert());
				}
				catch (const std::exception& ex)
				{
					std::wcerr
						<< "Failed to delete temp cert: "
						<< ex.what();
				}
			}
				
			TempCertImport(CertStore& store, Certificate& toImport)
				: m_store(store)
			{
				m_store.ImportCert(toImport.GetCert());
				m_thumbprint = toImport.GetSignature();
			}

		protected:
			CertStore m_store;
			std::wstring m_thumbprint;
	};
}

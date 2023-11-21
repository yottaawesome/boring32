export module boring32.crypto:tempcertimport;
import std;
import std.compat;
import :certificate;
import :certstore;

export namespace Boring32::Crypto
{
	class TempCertImport final
	{
		public:
			~TempCertImport()
			{
				Close();
			}
				
			TempCertImport(
				CertStore& store, 
				Certificate& toImport, 
				const CertAddDisposition disposition = CertAddDisposition::AddNewer
			) : m_store(store), m_cert(toImport)
			{
				m_store.AddCertificate(toImport.GetCert(), disposition);
			}

		public:
			void Close() noexcept try
			{
				m_store.DeleteCert(m_cert.GetCert());
			}
			catch (const std::exception& ex)
			{
				std::wcerr
					<< "Failed to delete temp cert: "
					<< ex.what();
			}

		private:
			CertStore m_store;
			Certificate m_cert;
	};
}

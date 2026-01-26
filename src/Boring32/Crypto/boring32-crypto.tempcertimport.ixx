export module boring32:crypto.tempcertimport;
import std;
import boring32.win32;
import :crypto.certificate;
import :crypto.certstore;

export namespace Boring32::Crypto
{
	struct TempCertImport final
	{
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

		void Close() noexcept 
		try
		{
			m_store.DeleteCert(m_cert.GetCert());
		}
		catch (const std::exception& ex)
		{
			std::wcerr << "Failed to delete temp cert: " << ex.what();
		}

		private:
		CertStore m_store;
		Certificate m_cert;
	};
}

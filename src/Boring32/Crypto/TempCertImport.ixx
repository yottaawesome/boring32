export module Boring32:Crypto.TempCertImport;
import std;
import :Win32;
import :Crypto.Certificate;
import :Crypto.CertStore;
import :Strings;

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
			CertAddDisposition disposition = CertAddDisposition::AddNewer
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
			std::wcerr << std::format(L"Failed to delete temp cert: {}\n", Strings::ConvertString(ex.what()));
		}

	private:
		CertStore m_store;
		Certificate m_cert;
	};
}

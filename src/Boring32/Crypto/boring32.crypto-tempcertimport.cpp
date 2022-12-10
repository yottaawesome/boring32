module;

#include <iostream>
#include <string>

module boring32.crypto:tempcertimport;

namespace Boring32::Crypto
{
	TempCertImport::~TempCertImport()
	{
		try
		{
			Certificate certToDelete = m_store.GetCertByByBase64Signature(m_thumbprint);
			m_store.DeleteCert(certToDelete.GetCert());
		}
		catch (const std::exception& ex)
		{
			std::wcerr 
				<< __FUNCSIG__
				<< ": Failed to delete temp cert: "
				<< ex.what();
		}
	}

	TempCertImport::TempCertImport(CertStore& store, Certificate& toImport)
		: m_store(store)
	{
		m_store.ImportCert(toImport.GetCert());
		m_thumbprint = toImport.GetSignature();
	}
}
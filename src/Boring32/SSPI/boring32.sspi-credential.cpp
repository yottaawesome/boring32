module;

#include <source_location>;

module boring32.sspi:credential;
import boring32.error;

namespace Boring32::SSPI
{
	Credential::Credential()
	{
		Create();
	}

	void Credential::Create()
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-schannel_cred
		// Deprecated
		SCHANNEL_CRED channelCred {
			.dwVersion = SCHANNEL_CRED_VERSION,
			.grbitEnabledProtocols = SP_PROT_TLS1,
			.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION
			// The SCH_CRED_MANUAL_CRED_VALIDATION flag is specified because
			// this sample verifies the server certificate manually.
			// Applications that expect to run on WinNT, Win9x, or WinME
			// should specify this flag and also manually verify the server
			// certificate. Applications running on newer versions of Windows can
			// leave off this flag, in which case the InitializeSecurityContext
			// function will validate the server certificate automatically.
		};
		TimeStamp tsExpiry;

		// https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--general
		CredentialUniquePtr creds(new CredHandle{0});
		SECURITY_STATUS status = AcquireCredentialsHandleW(
			nullptr,
			const_cast<wchar_t*>(UNISP_NAME_W),
			SECPKG_CRED_OUTBOUND,
			nullptr,
			&channelCred,
			nullptr,
			nullptr,
			creds.get(),
			&tsExpiry
		);
		if (status != SEC_E_OK)
		{
			throw Error::Boring32Error("AcquireCredentialsHandleW() failed");
		}
		m_credHandle = std::move(creds);
	}
}
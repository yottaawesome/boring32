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
			throw Error::Boring32Error(
				"AcquireCredentialsHandleW() failed with code {:#X}",
				std::source_location::current(),
				status
			);
		}
		m_credHandle = std::move(creds);
	}
}
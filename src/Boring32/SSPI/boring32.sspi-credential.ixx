export module boring32.sspi:credential;
import <memory>;
import <stacktrace>;
import <win32.hpp>;
import boring32.error;

namespace Boring32::SSPI
{
	struct CredentialDeleter final
	{
		void operator()(PCredHandle handle)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-freecredentialshandle
			FreeCredentialsHandle(handle);
		}
	};
	using CredentialUniquePtr = std::unique_ptr<CredHandle, CredentialDeleter>;

	export class Credential
	{
		public:
			virtual ~Credential() = default;
			Credential(const Credential&) = delete;
			Credential(Credential&&) noexcept = default;
			Credential()
			{
				Create2();
			}

		public:
			virtual Credential& operator=(const Credential&) = delete;
			virtual Credential& operator=(Credential&&) noexcept = default;

		protected:
			virtual void Create()
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-schannel_cred
				// Original version, deprecated structure
				SCHANNEL_CRED channelCred{
					.dwVersion = SCHANNEL_CRED_VERSION,
					.grbitEnabledProtocols = SP_PROT_TLS1,
					.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION
				};
				TimeStamp tsExpiry;

				// https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--general
				CredentialUniquePtr creds(new CredHandle{ 0 });
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
						std::stacktrace::current(),
						status
					);
				}
				m_credHandle = std::move(creds);
			}

			virtual void Create2()
			{
				// Non-deprecated version
				// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
				SCH_CREDENTIALS channelCred{
					.dwVersion = SCH_CREDENTIALS_VERSION,
					.dwFlags = SCH_CRED_NO_DEFAULT_CREDS | SCH_CRED_MANUAL_CRED_VALIDATION
				};
				TimeStamp tsExpiry;

				// https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--general
				CredentialUniquePtr creds(new CredHandle{ 0 });
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
						std::stacktrace::current(),
						status
					);
				}
				m_credHandle = std::move(creds);
			}

		protected:
			CredentialUniquePtr m_credHandle;
	};
}
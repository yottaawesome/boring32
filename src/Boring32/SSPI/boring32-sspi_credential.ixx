export module boring32:sspi_credential;
import boring32.shared;
import :error;

namespace Boring32::SSPI
{
	struct CredentialDeleter final
	{
		void operator()(Win32::PCredHandle handle)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/sspi/nf-sspi-freecredentialshandle
			Win32::FreeCredentialsHandle(handle);
		}
	};
	using CredentialUniquePtr = std::unique_ptr<Win32::CredHandle, CredentialDeleter>;

	export struct Credential final
	{
		~Credential() = default;
		Credential(const Credential&) = delete;
		Credential(Credential&&) noexcept = default;
		Credential()
		{
			Create2();
		}

		Credential& operator=(const Credential&) = delete;
		Credential& operator=(Credential&&) noexcept = default;

		private:
		void Create()
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-schannel_cred
			// Original version, deprecated structure
			Win32::SCHANNEL_CRED channelCred{
				.dwVersion = Win32::_SCHANNEL_CRED_VERSION,
				.grbitEnabledProtocols = Win32::_SP_PROT_TLS1,
				.dwFlags = Win32::_SCH_CRED_NO_DEFAULT_CREDS | Win32::_SCH_CRED_MANUAL_CRED_VALIDATION
			};
			Win32::TimeStamp tsExpiry;

			// https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--general
			CredentialUniquePtr creds(new Win32::CredHandle{ 0 });
			Win32::SECURITY_STATUS status = Win32::AcquireCredentialsHandleW(
				nullptr,
				const_cast<wchar_t*>(Win32::_UNISP_NAME_W),
				Win32::_SECPKG_CRED_OUTBOUND,
				nullptr,
				&channelCred,
				nullptr,
				nullptr,
				creds.get(),
				&tsExpiry
			);
			if (status != Win32::Sec_E_OK)
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

		void Create2()
		{
			// Non-deprecated version
			// https://learn.microsoft.com/en-us/windows/win32/api/schannel/ns-schannel-sch_credentials
			Win32::SCH_CREDENTIALS channelCred{
				.dwVersion = Win32::_SCH_CREDENTIALS_VERSION,
				.dwFlags = Win32::_SCH_CRED_NO_DEFAULT_CREDS | Win32::_SCH_CRED_MANUAL_CRED_VALIDATION
			};
			Win32::TimeStamp tsExpiry;

			// https://learn.microsoft.com/en-us/windows/win32/secauthn/acquirecredentialshandle--general
			CredentialUniquePtr creds(new CredHandle{ 0 });
			Win32::SECURITY_STATUS status = Win32::AcquireCredentialsHandleW(
				nullptr,
				const_cast<wchar_t*>(Win32::_UNISP_NAME_W),
				Win32::_SECPKG_CRED_OUTBOUND,
				nullptr,
				&channelCred,
				nullptr,
				nullptr,
				creds.get(),
				&tsExpiry
			);
			if (status != Win32::Sec_E_OK)
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

		CredentialUniquePtr m_credHandle;
	};
}
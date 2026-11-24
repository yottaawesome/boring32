export module boring32:security.credentials;
import std;
import :win32;
import :raii;
import :error;

namespace Boring32::Security::Credentials
{
    using CredentialUniquePtr = 
        RAII::UniquePtr<Win32::Credentials::CREDENTIALW, Win32::Credentials::CredFree>;
}

// https://learn.microsoft.com/en-us/windows/win32/secauthn/credentials-management
export namespace Boring32::Security::Credentials
{
	void Add(
        const std::wstring& account,
        const std::wstring& username,
        const std::wstring& password,
		Win32::Credentials::Types type = Win32::Credentials::Types::Generic,
		Win32::Credentials::Persist persist = Win32::Credentials::Persist::LocalMachine
    )
	{
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credentialw
        auto cred = Win32::Credentials::CREDENTIALW{
            .Type = type,
            .TargetName = const_cast<wchar_t*>(account.data()),
            .CredentialBlobSize = static_cast<Win32::DWORD>(1 + password.size()),
            .CredentialBlob =
                reinterpret_cast<Win32::LPBYTE>(const_cast<wchar_t*>(password.data())),
            .Persist = persist,
            .UserName = const_cast<wchar_t*>(username.data())
        };

        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-credwritew
        if (not Win32::Credentials::CredWriteW(&cred, 0))
            throw Error::Win32Error{ Win32::GetLastError(), "CredWriteW() failed to add credential."};
	}

    auto Delete(
        const std::wstring& account,
        Win32::Credentials::Types type = Win32::Credentials::Types::Generic
    ) -> bool
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-creddeletew
        auto success = 
            Win32::Credentials::CredDeleteW(
                account.data(),
                type,
                0
            );
        if (success)
			return true;

        auto lastError = Win32::GetLastError();
        return lastError == Win32::ErrorCodes::NotFound
			? false
			: throw Error::Win32Error{lastError, "CredDeleteW() failed to delete credential."};
	}

    struct Credential
    {
        std::wstring Username;
        std::wstring Password;
	};

    auto Read(
        const std::wstring& account,
        Win32::Credentials::Types type = Win32::Credentials::Types::Generic
    ) -> std::optional<Credential>
    {
        auto pcred = (Win32::Credentials::CREDENTIALW*)nullptr;
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-credreadw
        auto success = 
            Win32::Credentials::CredReadW(
                account.data(),
                type,
                0,
                &pcred
            );
        if (not success)
        {
            auto lastError = Win32::GetLastError();
            return lastError == Win32::ErrorCodes::NotFound
                ? std::nullopt
                : throw Error::Win32Error{lastError, "CredReadW() failed to read credential."};
        }
        auto cred = CredentialUniquePtr{pcred};

        return Credential{
            .Username = cred->UserName ? cred->UserName : L"",
            .Password = cred->CredentialBlob 
                ? std::wstring{ reinterpret_cast<wchar_t*>(cred->CredentialBlob), cred->CredentialBlobSize / sizeof(wchar_t) } 
                : L""
		};
    }
}

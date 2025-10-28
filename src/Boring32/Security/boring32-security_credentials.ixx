export module boring32:security_credentials;
import std;
import boring32.win32;
import :raii;
import :error;

namespace Boring32::Security::Credentials
{
    using CredentialUniquePtr = 
        RAII::UniquePtr <Win32::Credentials::CREDENTIALW, Win32::Credentials::CredFree>;
}

// https://learn.microsoft.com/en-us/windows/win32/secauthn/credentials-management
export namespace Boring32::Security::Credentials
{
	void Add(
        const std::wstring& account,
        const std::wstring& username,
        const std::wstring& password
    )
	{
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credentialw
        Win32::Credentials::CREDENTIALW cred { 
            .Type = Win32::Credentials::Types::Generic,
            .TargetName = const_cast<wchar_t*>(account.data()),
            .CredentialBlobSize = static_cast<Win32::DWORD>(1 + password.size()),
            .CredentialBlob =
                reinterpret_cast<Win32::LPBYTE>(const_cast<wchar_t*>(password.data())),
            .Persist = Win32::Credentials::CredPersistLocalMachine,
            .UserName = const_cast<wchar_t*>(username.data())
        };

        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-credwritew
        if (not Win32::Credentials::CredWriteW(&cred, 0))
        {
			const auto lastError = Win32::GetLastError();
            throw Error::Win32Error(lastError, "CredWriteW() failed to add credential.");
        }
	}

    auto Delete(const std::wstring& account) -> bool
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-creddeletew
        bool success = Win32::Credentials::CredDeleteW(
            account.data(),
            Win32::Credentials::Types::Generic,
            0
        );
        if (success)
			return true;

        const auto lastError = Win32::GetLastError();
        return lastError == Win32::ErrorCodes::NotFound
			? false
			: throw Error::Win32Error(lastError, "CredDeleteW() failed to delete credential.");
	}

    struct Credential
    {
        std::wstring Username;
        std::wstring Password;
	};

    auto Read(
        const std::wstring& account,
        std::wstring& outUsername,
        std::wstring& outPassword
    ) -> Credential
    {
        Win32::Credentials::CREDENTIALW* pcred = nullptr;
        // https://learn.microsoft.com/en-us/windows/win32/api/wincred/nf-wincred-credreadw
        bool success = Win32::Credentials::CredReadW(
            account.data(),
            Win32::Credentials::Types::Generic,
            0,
            &pcred
        );
        if (not success)
        {
            const auto lastError = Win32::GetLastError();
            throw Error::Win32Error(lastError, "CredReadW() failed to read credential.");
        }
        CredentialUniquePtr cred(pcred);

        return Credential{
            .Username = cred->UserName ? cred->UserName : L"",
            .Password = cred->CredentialBlob 
                ? std::wstring{ reinterpret_cast<wchar_t*>(cred->CredentialBlob), cred->CredentialBlobSize / sizeof(wchar_t) } 
                : L""
		};
    }
}

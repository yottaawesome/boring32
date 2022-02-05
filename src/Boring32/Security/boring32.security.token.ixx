module;
#include <string>
#include <Windows.h>

export module boring32.security.token;
import boring32.security.constants;
import boring32.raii.win32handle;

export namespace Boring32::Security
{
	class Token
	{
		public:
			virtual ~Token();
			Token();
			Token(const Token& other);
			Token(Token&& other) noexcept;
			Token(const DWORD desiredAccess);
			Token(const HANDLE processHandle, const DWORD desiredAccess);
			Token(const HANDLE token, const bool ownOrDuplicate);

		public:
			virtual Token& operator=(const Token& other);
			virtual Token& operator=(Token&& other) noexcept;

		public:
			virtual void Close();
			virtual Raii::Win32Handle GetToken() const noexcept;
			/// <summary>
			/// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
			/// </summary>
			virtual void AdjustPrivileges(const std::wstring& privilege, const bool enabled);
			virtual void SetIntegrity(const Constants::GroupIntegrity integrity);

		protected:
			virtual Token& Copy(const Token& other);
			virtual Token& Move(Token& other) noexcept;

		protected:
			Raii::Win32Handle m_token;
	};
}
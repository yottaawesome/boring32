#pragma once
#include <Windows.h>
#include "../Raii/Win32Handle.hpp"
#include "Constants.hpp"

namespace Boring32::Security
{
	class Token
	{
		public:
			virtual ~Token();
			Token();
			Token(const DWORD desiredAccess);
			Token(HANDLE processHandle, const DWORD desiredAccess);
			Token(const HANDLE token, const bool ownOrDuplicate);

		public:
			virtual Raii::Win32Handle GetToken() const noexcept;
			/// <summary>
			/// https://docs.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
			/// </summary>
			virtual void AdjustPrivileges(const std::wstring& privilege, const bool enabled);
			virtual void SetIntegrity(const Constants::GroupIntegrity integrity);

		protected:

		protected:
			Raii::Win32Handle m_token;
	};
}
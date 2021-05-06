#pragma once
#include <string>
#include <map>
#include <Windows.h>
#include "../Raii/Win32Handle.hpp"

namespace Boring32::Security
{
	enum class GroupIntegrity
	{
		Low,
		Medium,
		MediumPlus,
		High,
		System,
		ProtectedProcess,
		SecureProcess
	};

	//https://docs.microsoft.com/en-US/troubleshoot/windows-server/identity/security-identifiers-in-windows
	static const std::map<GroupIntegrity, std::wstring> Integrities{
		{GroupIntegrity::Low, L"S-1-16-4096"},
		{GroupIntegrity::Medium, L"S-1-16-8192"},
		{GroupIntegrity::MediumPlus, L"S-1-16-8448"},
		{GroupIntegrity::High, L"S-1-16-12288"},
		{GroupIntegrity::System, L"S-1-16-16384"},
		{GroupIntegrity::ProtectedProcess, L"S-1-16-20480"},
		{GroupIntegrity::SecureProcess, L"S-1-16-28672"}
	};

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
			virtual void SetIntegrity(const GroupIntegrity integrity);

		protected:

		protected:
			Raii::Win32Handle m_token;
	};
}
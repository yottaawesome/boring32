#include "pch.hpp"
#include <sddl.h>
#include "include/Error/Win32Error.hpp"
#include "include/Security/Token.hpp"
#include "include/Security/SecurityFunctions.hpp"

namespace Boring32::Security
{
	Token::~Token()
	{
		m_token = nullptr;
	}

	Token::Token() { }

	Token::Token(const DWORD desiredAccess)
	{
		m_token = GetProcessToken(GetCurrentProcess(), desiredAccess);
	}

	Token::Token(HANDLE processHandle, const DWORD desiredAccess)
	{
		m_token = GetProcessToken(processHandle, desiredAccess);
	}

	Token::Token(const HANDLE token, const bool ownOrDuplicate)
	{
		if (token == nullptr)
			throw std::invalid_argument(__FUNCSIG__ ": token cannot be null");

		if (ownOrDuplicate)
		{
			m_token = Raii::Win32Handle(token);
			return;
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-duplicatetokenex
		const bool succeeded = DuplicateTokenEx(
			token,
			0,
			nullptr,
			SecurityImpersonation,
			TokenPrimary,
			&m_token
		);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__ ": DuplicateTokenEx() failed");
	}

	Raii::Win32Handle Token::GetToken() const noexcept
	{
		return m_token;
	}

	void Token::AdjustPrivileges(const std::wstring& privilege, const bool enabled)
	{
		Security::AdjustPrivileges(m_token.GetHandle(), privilege, enabled);
	}

	void Token::SetIntegrity(const GroupIntegrity integrity)
	{
		const std::wstring& integritySid = Integrities.at(integrity);
		PSID pIntegritySid = nullptr;
		bool succeeded = ConvertStringSidToSidW(integritySid.c_str(), &pIntegritySid);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__": ConvertStringSidToSidW() failed", GetLastError());

		TOKEN_MANDATORY_LABEL tml = { 0 };
		tml.Label.Attributes = SE_GROUP_INTEGRITY;
		tml.Label.Sid = pIntegritySid;
		succeeded = SetTokenInformation(
			m_token.GetHandle(),
			TokenIntegrityLevel,
			&tml,
			sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)
		);
		LocalFree(pIntegritySid);
		if (succeeded == false)
			throw Error::Win32Error(__FUNCSIG__": SetTokenInformation() failed", GetLastError());
	}
}
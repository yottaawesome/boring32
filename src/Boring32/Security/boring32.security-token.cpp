module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>
#include <sddl.h>

module boring32.security:token;
import boring32.error;
import :functions;

namespace Boring32::Security
{
	Token::~Token()
	{
		Close();
	}

	Token::Token() { }

	Token::Token(const Token& other)
		: Token()
	{
		Copy(other);
	}

	Token::Token(Token&& other) noexcept
		: Token()
	{
		Move(other);
	}

	Token::Token(const DWORD desiredAccess)
	{
		m_token = GetProcessToken(GetCurrentProcess(), desiredAccess);
	}

	Token::Token(const HANDLE processHandle, const DWORD desiredAccess)
	{
		m_token = GetProcessToken(processHandle, desiredAccess);
	}

	Token::Token(const HANDLE token, const bool ownOrDuplicate)
	{
		if (!token)
			throw Error::ErrorBase<std::invalid_argument>(std::source_location::current(), "Token cannot be null");

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
		if (!succeeded)
			throw Error::Win32Error(std::source_location::current(), "DuplicateTokenEx() failed", GetLastError());
	}

	Token& Token::operator=(const Token& other)
	{
		return Copy(other);
	}

	Token& Token::operator=(Token&& other) noexcept
	{
		return Move(other);
	}

	void Token::Close()
	{
		m_token = nullptr;
	}

	Raii::Win32Handle Token::GetToken() const noexcept
	{
		return m_token;
	}

	void Token::AdjustPrivileges(const std::wstring& privilege, const bool enabled)
	{
		Security::AdjustPrivileges(m_token.GetHandle(), privilege, enabled);
	}

	void Token::SetIntegrity(const Constants::GroupIntegrity integrity)
	{
		Security::SetIntegrity(m_token.GetHandle(), integrity);
	}

	Token& Token::Copy(const Token& other)
	{
		if (&other == this)
			return *this;
		
		Close();
		if (other.m_token)
		{
			const bool succeeded = DuplicateTokenEx(
				other.m_token.GetHandle(),
				0,
				nullptr,
				SecurityImpersonation,
				TokenPrimary,
				&m_token
			);
			if (!succeeded)
				throw Error::Win32Error(std::source_location::current(), "DuplicateTokenEx() failed", GetLastError());
		}

		return *this;
	}

	Token& Token::Move(Token& other) noexcept
	{
		if (&other == this)
			return *this;

		Close();
		m_token = std::move(other.m_token);
		
		return *this;
	}
}
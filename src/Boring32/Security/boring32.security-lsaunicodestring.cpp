module;

#include <string>

module boring32.security:lsaunicodestring;

namespace Boring32::Security
{
	LSAUnicodeString::LSAUnicodeString(std::wstring str)
		: m_string(std::move(str))
	{
		Init();
	}

	LSAUnicodeString::LSAUnicodeString(const LSAUnicodeString& other)
		: m_string(other.m_string)
	{
		Init();
	}

	LSAUnicodeString::LSAUnicodeString(LSAUnicodeString&& other)
		: m_string(std::move(other.m_string))
	{
		Init();
	}

	LSAUnicodeString& LSAUnicodeString::operator=(const LSAUnicodeString& other)
	{
		m_string = other.m_string;
		Init();
		return *this;
	}
	
	LSAUnicodeString& LSAUnicodeString::operator=(LSAUnicodeString&& other) 
	{
		m_string = std::move(other.m_string);
		Init();
		return *this;
	}

	LSAUnicodeString::operator LSA_UNICODE_STRING* () noexcept
	{
		return &m_lsaStr;
	}

	void LSAUnicodeString::Init()
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/lsalookup/ns-lsalookup-lsa_unicode_string
		m_lsaStr = {
			.Length = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.MaximumLength = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.Buffer = m_string.data()
		};
	}
}
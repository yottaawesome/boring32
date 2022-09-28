module;

#include <string>
#include <Windows.h>
#include <Ntsecapi.h>

module boring32.security:lsaunicodestring;

namespace Boring32::Security
{
	LSAUnicodeString::LSAUnicodeString(std::wstring str)
		: m_string(std::move(str))
	{
		// https://learn.microsoft.com/en-us/windows/win32/api/lsalookup/ns-lsalookup-lsa_unicode_string
		m_lsaStr = {
			.Length = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.MaximumLength = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.Buffer = m_string.data()
		};
	}
	
	LSAUnicodeString::operator LSA_UNICODE_STRING* () noexcept
	{
		return &m_lsaStr;
	}
}
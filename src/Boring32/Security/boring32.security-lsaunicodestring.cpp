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
		m_lsaStr = {
			.Length = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.MaximumLength = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
			.Buffer = const_cast<wchar_t*>(m_string.c_str())
		};
	}
	
	LSAUnicodeString::operator LSA_UNICODE_STRING* ()
	{
		return &m_lsaStr;
	}
}
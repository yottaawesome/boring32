module;

#include <string>
#include "include/Strings/Strings.hpp"

module boring32.strings;

namespace Boring32::Strings
{
	Utf16String::~Utf16String(){}
	Utf16String::Utf16String() {}
	Utf16String::Utf16String(const std::string& str)
		: m_string(str)
	{
		m_wstring = Strings::ConvertString(str);
	}
	Utf16String::Utf16String(const std::wstring& str)
		: m_wstring(str)
	{
		m_string = Strings::ConvertString(str);

	}
	Utf16String::Utf16String(const char* str)
		: m_string(str)
	{
		m_wstring = Strings::ConvertString(str);
	}
	Utf16String::Utf16String(const wchar_t* str)
		: m_wstring(str)
	{
		m_string = Strings::ConvertString(str);
	}

	Utf16String& Utf16String::operator=(const std::string& str)
	{
		m_string = str;
		m_wstring = Strings::ConvertString(str);
		return *this;
	}

	Utf16String& Utf16String::operator=(const std::wstring& str)
	{
		m_wstring = str;
		m_string = Strings::ConvertString(str);
		return *this;
	}

	Utf16String& Utf16String::operator=(const char* str)
	{
		m_string = str;
		m_wstring = Strings::ConvertString(str);
		return *this;
	}

	Utf16String& Utf16String::operator=(const wchar_t* str)
	{
		m_wstring = str;
		m_string = Strings::ConvertString(str);
		return *this;
	}

	Utf16String::operator const char*() const noexcept
	{
		return m_string.c_str();
	}
	Utf16String::operator const wchar_t*() const noexcept
	{
		return m_wstring.c_str();
	}

	const char* Utf16String::ToCString() const noexcept
	{
		return m_string.c_str();
	}

	const wchar_t* Utf16String::ToWCString() const noexcept
	{
		return m_wstring.c_str();
	}

	std::string Utf16String::ToString() const noexcept
	{
		return m_string;
	}
	
	std::wstring Utf16String::ToWString() const noexcept
	{
		return m_wstring;
	}

	const std::string& Utf16String::String() const noexcept
	{
		return m_string;
	}

	const std::wstring& Utf16String::WString() const noexcept
	{
		return m_wstring;
	}

	size_t Utf16String::Size() const noexcept
	{
		return m_string.size();
	}
}
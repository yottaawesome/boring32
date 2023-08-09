export module boring32.security:lsaunicodestring;
import std;

import <win32.hpp>;

namespace Boring32::Security
{
	class LSAUnicodeString final
	{
		public:
			LSAUnicodeString(std::wstring str)
				: m_string(std::move(str))
			{
				Init();
			}

			LSAUnicodeString(const LSAUnicodeString& other)
				: m_string(other.m_string)
			{
				Init();
			}

			LSAUnicodeString(LSAUnicodeString&& other)
				: m_string(std::move(other.m_string))
			{
				Init();
			}

		public:
			LSAUnicodeString& operator=(const LSAUnicodeString& other)
			{
				m_string = other.m_string;
				Init();
				return *this;
			}

			LSAUnicodeString& operator=(LSAUnicodeString&& other)
			{
				m_string = std::move(other.m_string);
				Init();
				return *this;
			}

			LSA_UNICODE_STRING* Get() noexcept
			{
				return &m_lsaStr;
			}

		public:
			// Weirdly, this fails to compile in 17.5.0
			// No error if only LSA_UNICODE_STRING is returned (no pointer) 
			//operator LSA_UNICODE_STRING* ();

		private:
			void Init()
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/lsalookup/ns-lsalookup-lsa_unicode_string
				m_lsaStr = {
					.Length = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
					.MaximumLength = static_cast<unsigned short>(m_string.size() * sizeof(wchar_t)),
					.Buffer = m_string.data()
				};
			}

		private:
			// We need this so the buffer pointed to by m_lsaStr is valid
			// for temporary strings.
			std::wstring m_string;
			LSA_UNICODE_STRING m_lsaStr;
	};
}
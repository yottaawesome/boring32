module;

#include <string>
#include <Windows.h>
#include <Ntsecapi.h>

export module boring32.security:lsaunicodestring;

namespace Boring32::Security
{
	class LSAUnicodeString final
	{
		public:
			LSAUnicodeString(std::wstring str);
			// Disable copy and move semantics for now.
			LSAUnicodeString(const LSAUnicodeString&) = delete;
			LSAUnicodeString(LSAUnicodeString&&) = delete;

		public:
			LSAUnicodeString& operator=(const LSAUnicodeString&) = delete;
			LSAUnicodeString& operator=(LSAUnicodeString&&) = delete;

		public:
			operator LSA_UNICODE_STRING* () noexcept;

		private:
			// We need this so the buffer pointed to by m_lsaStr is valid
			// for temporary strings.
			std::wstring m_string;
			LSA_UNICODE_STRING m_lsaStr;
	};
}
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

		public:
			operator LSA_UNICODE_STRING* ();

		private:
			std::wstring m_string;
			LSA_UNICODE_STRING m_lsaStr;
	};
}
export module boring32.security:lsaunicodestring;
import <string>;
import <win32.hpp>;

namespace Boring32::Security
{
	class LSAUnicodeString final
	{
		public:
			LSAUnicodeString(std::wstring str);
			LSAUnicodeString(const LSAUnicodeString& other);
			LSAUnicodeString(LSAUnicodeString&& other);

		public:
			LSAUnicodeString& operator=(const LSAUnicodeString& other);
			LSAUnicodeString& operator=(LSAUnicodeString&& other);

		public:
			operator LSA_UNICODE_STRING* () noexcept;

		private:
			void Init();

		private:
			// We need this so the buffer pointed to by m_lsaStr is valid
			// for temporary strings.
			std::wstring m_string;
			LSA_UNICODE_STRING m_lsaStr;
	};
}
module;

#include <string>

export module boring32.strings;

export namespace Boring32::Strings
{
	class Utf16String
	{
		// Constructors
		public:
			virtual ~Utf16String();
			Utf16String();
			Utf16String(const std::string& str);
			Utf16String(const std::wstring& str);
			Utf16String(const char* str);
			Utf16String(const wchar_t* str);

		// Operators
		public:
			virtual Utf16String& operator=(const std::string& str);
			virtual Utf16String& operator=(const std::wstring& str);
			virtual Utf16String& operator=(const char* str);
			virtual Utf16String& operator=(const wchar_t* str);
			virtual operator const char*() const noexcept;
			virtual operator const wchar_t*() const noexcept;

		// API
		public:
			virtual const char* ToCString() const noexcept;
			virtual const wchar_t* ToWCString() const noexcept;
			virtual std::string ToString() const noexcept;
			virtual std::wstring ToWString() const noexcept;
			virtual const std::string& String() const noexcept;
			virtual const std::wstring& WString() const noexcept;
			virtual size_t Size() const noexcept;

		// Members
		protected:
			std::string m_string;
			std::wstring m_wstring;
	};
}
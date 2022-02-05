module;

#include <string>
#include <vector>

export module boring32.strings;

export namespace Boring32::Strings
{
	std::string ConvertString(const std::wstring_view wstr);
	std::wstring ConvertString(const std::string_view str);
	std::vector<std::wstring> TokeniseString(
		const std::wstring& stringToTokenise,
		const std::wstring& delimiter
	);
	std::wstring Replace(
		std::wstring source,
		const std::wstring& from,
		const std::wstring& to
	);
	std::wstring Erase(std::wstring source, const wchar_t what);
	bool DoCaseInsensitiveMatch(std::string str1, std::string str2);
	bool DoCaseInsensitiveMatch(std::wstring str1, std::wstring str2);

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
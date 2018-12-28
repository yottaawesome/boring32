#pragma once
#include <string>
#include <Windows.h>

using std::string;
using std::wstring;

namespace Win32Common
{
	class GenericStrings
	{
		public:
			static string ConvertWStringToString(const wstring& str);
			static wstring ConvertStringToWString(const string& str);

			static char* ConvertWCharToChar(const wchar_t* str, const size_t strLength);
			static wchar_t* ConvertCharToWChar(const char* str, const size_t strLength);
	};

	class Win32Strings
	{
		public:
			static string ConvertWStringToString(const wstring& wstr);
			static wstring ConvertStringToWString(const string& str);
	};

	class IDynamicLinkLibrary
	{
		public:
			virtual const wstring& GetPath() = 0;
			virtual const HMODULE GetHandle() = 0;
			virtual ~IDynamicLinkLibrary() = 0;
	};
}

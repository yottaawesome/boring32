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

			//static char* ConvertWCharToChar(const wchar_t* str, const size_t strLength);
			//static wchar_t* ConvertCharToWChar(const char* str, const size_t strLength);
	};

	class Win32Strings
	{
		public:
			static string ConvertWStringToString(const wstring& wstr);
			static wstring ConvertStringToWString(const string& str);
	};

	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const wstring& path);
			virtual const wstring& GetPath();
			virtual const HMODULE GetHandle();
			virtual void* Resolve(const wstring& path);
			virtual ~DynamicLinkLibrary();

		protected:
			const wstring path;
			HMODULE libraryHandle;
	};
}

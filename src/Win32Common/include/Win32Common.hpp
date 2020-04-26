#pragma once
#include <string>
#include <Windows.h>

namespace Win32Utils
{
	class GenericStrings
	{
		public:
			static std::string ConvertWStringToString(const std::wstring& str);
			static std::wstring ConvertStringToWString(const std::string& str);

			//static char* ConvertWCharToChar(const wchar_t* str, const size_t strLength);
			//static wchar_t* ConvertCharToWChar(const char* str, const size_t strLength);
	};

	std::string ConvertWStringToString(const std::wstring& wstr);
	std::wstring ConvertStringToWString(const std::string& str);

	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const std::wstring& path);
			virtual ~DynamicLinkLibrary();
			virtual const std::wstring& GetPath();
			virtual const HMODULE GetHandle();
			virtual void* Resolve(const std::wstring& path);

		protected:
			const std::wstring path;
			HMODULE libraryHandle;
	};
}

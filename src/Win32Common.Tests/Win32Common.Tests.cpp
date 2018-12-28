#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Common.h"

int main()
{
	// TODO: use Catch2 unit testing framework

	const wchar_t* test1 = L"Test1\0";
	const char* test2 = "Test2\0";

	std::cout << Win32Common::GenericStrings::ConvertWCharToChar(test1, sizeof(test1)/sizeof(wchar_t)) << "\n";
	std::wcout << Win32Common::GenericStrings::ConvertCharToWChar(test2, sizeof(test2)/sizeof(char)) << L"\n";
	std::cout << Win32Common::GenericStrings::ConvertWStringToString(test1) << "\n";
	std::wcout << Win32Common::GenericStrings::ConvertStringToWString(test2) << "\n";
	std::cout << Win32Common::Win32Strings::ConvertWStringToString(test1) << "\n";
	std::wcout << Win32Common::Win32Strings::ConvertStringToWString(test2) << "\n";
}

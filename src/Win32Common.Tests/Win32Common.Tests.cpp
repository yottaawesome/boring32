#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Common.h"
#include <dbghelp.h>

using Win32Common::DynamicLinkLibrary;

void testConversions()
{
	const wchar_t test1[] = L"Test1\0";
	const char test2[] = "Test2\0";
	std::cout << Win32Common::GenericStrings::ConvertWStringToString(test1) << "\n";
	std::wcout << Win32Common::GenericStrings::ConvertStringToWString(test2) << "\n";
	std::cout << Win32Common::Win32Strings::ConvertWStringToString(test1) << "\n";
	std::wcout << Win32Common::Win32Strings::ConvertStringToWString(test2) << "\n";
}

void testLibraryLoad()
{
	DynamicLinkLibrary d(L"Onyx32.Filesystem.dll");
	void* mainFunc = d.Resolve(L"GetMainInterface");
	if (mainFunc == nullptr)
		throw new std::runtime_error("Could not resolve function");
}

int main()
{
	testConversions();
	testLibraryLoad();
}

#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Common.h"
#include <dbghelp.h>

using Win32Common::DynamicLinkLibrary;

void testConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<< Win32Common::Win32Strings::ConvertStringToWString(
			Win32Common::Win32Strings::ConvertWStringToString(test1))
		<< "\n";
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
	// todo: add a test framework like Catch2

	testConversions();
	testLibraryLoad();
}

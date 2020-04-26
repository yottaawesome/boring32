#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Common.hpp"
#include <dbghelp.h>

using Win32Utils::DynamicLinkLibrary;

void TestConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<< Win32Utils::ConvertStringToWString(
			Win32Utils::ConvertWStringToString(test1))
		<< "\n";
}

void TestLibraryLoad()
{
	DynamicLinkLibrary d(L"Onyx32.Filesystem.dll");
	void* mainFunc = d.Resolve(L"GetMainInterface");
	if (mainFunc == nullptr)
		throw new std::runtime_error("Could not resolve function");
}

int main(int argc, char** args)
{
	// todo: add a test framework like Catch2

	TestConversions();
	TestLibraryLoad();

	return 0;
}

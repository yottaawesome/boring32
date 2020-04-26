#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Utils.hpp"
#include <dbghelp.h>

void TestConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<< Win32Utils::Strings::ConvertStringToWString(
			Win32Utils::Strings::ConvertWStringToString(test1))
		<< "\n";
}

void TestLibraryLoad()
{
	Win32Utils::Process::DynamicLinkLibrary d(L"Onyx32.Filesystem.dll");
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

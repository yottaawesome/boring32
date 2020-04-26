#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Utils.hpp"
#include <dbghelp.h>

void TestMutex()
{
	Win32Utils::Process::Mutex m1(L"HelloMutex", true, false, false);
	m1.Lock(1000);
	m1.Unlock();

	Win32Utils::Process::Mutex m2(m1);

	Win32Utils::Process::Mutex m3(false, false);

	m2 = m3;
}

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

	TestMutex();
	TestConversions();
	//TestLibraryLoad();

	return 0;
}

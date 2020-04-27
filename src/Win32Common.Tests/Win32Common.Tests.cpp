#include "Win32Common.Tests.h"
#include <iostream>
#include "../Win32Common/include/Win32Utils.hpp"
#include <dbghelp.h>

void TestMutex()
{
	Win32Utils::IPC::Mutex m1(L"HelloMutex", true, false, false);
	m1.Lock(1000);
	m1.Unlock();

	Win32Utils::IPC::Mutex m2(m1);
	Win32Utils::IPC::Mutex m3(false, false);

	m2 = m3;
}

void TestConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<<	Win32Utils::Strings::ConvertStringToWString(
				Win32Utils::Strings::ConvertWStringToString(
					test1
				)
			)
		<< std::endl;
}

void TestMemoryMappedFile()
{
	Win32Utils::IPC::MemoryMappedFile m1(L"HelloMmf1", 1000, true, false);
	Win32Utils::IPC::MemoryMappedFile m2(m1);
	Win32Utils::IPC::MemoryMappedFile m3(L"HelloMmf3", 1000, true, false);
	m2 = m3;
}

void TestLibraryLoad()
{
	Win32Utils::Library::DynamicLinkLibrary d(L"Onyx32.Filesystem.dll");
	void* mainFunc = d.Resolve(L"GetMainInterface");
	if (mainFunc == nullptr)
		throw new std::runtime_error("Could not resolve function");
}

void TestAnonPipes()
{
	std::wstring whatToSend(L"Poo");
	Win32Utils::IPC::Pipe pipe(true, 512);
	pipe.Write(whatToSend);
	std::wstring response = pipe.ReadFromPipe();
	if (whatToSend != response)
		throw std::runtime_error("Failed to match input to output");

	Win32Utils::IPC::Pipe pipe2(pipe);
}

int main(int argc, char** args)
{
	// todo: add a test framework like Catch2

	TestMutex();
	TestConversions();
	TestMemoryMappedFile();
	TestAnonPipes();
	//TestLibraryLoad();

	return 0;
}

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <dbghelp.h>
#include "Boring32.Tests.h"
#include "../Boring32/include/Boring32.hpp"

#include "pathcch.h"
#pragma comment(lib, "Pathcch.lib")

void TestMutex()
{
	Boring32::Async::Mutex m1(L"HelloMutex", true, false, false);
	m1.Lock(1000);
	m1.Unlock();

	Boring32::Async::Mutex m2(m1);
	Boring32::Async::Mutex m3(false, false);

	m2 = m3;
	m2 = Boring32::Async::Mutex(L"Assignment", true, false, false);
}

void TestConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<<	Boring32::Strings::ConvertStringToWString(
				Boring32::Strings::ConvertWStringToString(
					test1
				)
			)
		<< std::endl;
}

void TestMemoryMappedFile()
{
	Boring32::Async::MemoryMappedFile m1(L"HelloMmf1", 1000, true, false);
	Boring32::Async::MemoryMappedFile m2(m1);
	Boring32::Async::MemoryMappedFile m3(L"HelloMmf3", 1000, true, false);
	m2 = m3;
}

void TestLibraryLoad()
{
	Boring32::Library::DynamicLinkLibrary d(L"Onyx32.Filesystem.dll");
	void* mainFunc = d.Resolve(L"GetMainInterface");
	if (mainFunc == nullptr)
		throw new std::runtime_error("Could not resolve function");
}

void TestAnonPipes()
{
	std::wstring msg1(L"message1");
	std::wstring msg2(L"message2");
	Boring32::Async::AnonymousPipe pipe(true, 512, L"||");
	pipe.DelimitedWrite(msg1);
	pipe.DelimitedWrite(msg2);

	auto response = pipe.DelimitedRead();
	std::wcout << response[0] << L" " << response[1] << std::endl;
	if (msg1 != response[0] || msg2 != response[1])
		throw std::runtime_error("Failed to match input to output");
}

int main(int argc, char** args)
{
	// todo: add a test framework like Catch2

	TestMutex();
	TestConversions();
	TestMemoryMappedFile();
	TestAnonPipes();
	//TestLibraryLoad();


	std::wstring directory;
	directory.resize(1024);
	GetModuleFileName(nullptr, &directory[0], directory.size());
	PathCchRemoveFileSpec(&directory[0], directory.size());
	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
	std::wstring filePath = directory+L"\\TestProcess.exe";

	Boring32::Async::AnonymousPipe childWrite(true, 2048, L"||");
	Boring32::Async::AnonymousPipe childRead(true, 2048, L"||");
	std::wstringstream ss;
	ss << "TestProcess.exe " << (int)childWrite.GetWrite() << L" " << (int)childRead.GetRead();
	//std::wcout << ss.str() << std::endl;

	Boring32::Async::Process p(filePath, ss.str(), directory, true);
	p.Start();

	childRead.DelimitedWrite(L"Hello from parent!");
	Sleep(1000);
	std::wcout << std::endl << childWrite.Read() << std::endl;

	//Boring32::WinHttp::HttpWebClient client(
	//	L"TestClientAgent", 
	//	L"127.0.0.1", 
	//	94873, 
	//	true, 
	//	{ L"application/json" },
	//	L"Content-Type: application/json"
	//);
	//client.Post(L"/some/object", "");

	return 0;
}

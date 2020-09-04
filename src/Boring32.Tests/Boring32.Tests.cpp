#include <Windows.h>
#include <iostream>
#include <sstream>
#include <dbghelp.h>
#include "Boring32.Tests.h"
#include "../Boring32/include/Boring32.hpp"

#include "pathcch.h"
#pragma comment(lib, "Pathcch.lib")

void print_exception_info(const std::exception& e)
{
	std::wcout << e.what() << std::endl;
	try 
	{
		rethrow_if_nested(e);
	}
	catch (const std::exception& ne)
	{
		print_exception_info(ne);
	}
}

void TestException()
{
	try
	{
		throw std::logic_error("first");
	}
	catch (const std::exception& ex)
	{
		try
		{
			std::throw_with_nested(std::logic_error("second"));
		}
		catch (const std::exception& ex2)
		{
			print_exception_info(ex);
		}
	}
}

void TestWaitableTime(int64_t relativeMillis)
{
	Boring32::Async::WaitableTimer timer(true, L"WaitableTimer", false, false);
	std::wcout << L"Timer set for " << relativeMillis << L" from now" << std::endl;
	timer.SetTimerInMillis(-relativeMillis, relativeMillis);
	timer.WaitOnTimer(INFINITE);
	timer.CancelTimer();
}

void TestSemaphore()
{
	Boring32::Async::Semaphore semaphore(L"Sem", false, 10, 10);
	semaphore.Acquire(2, INFINITE);
	if (semaphore.GetCurrentCount() != 8)
		throw std::runtime_error("Invalid error count");
	semaphore.Release(2);
	if (semaphore.GetCurrentCount() != 10)
		throw std::runtime_error("Invalid error count");
}

void TestMutex()
{
	Boring32::Async::Mutex m1(L"HelloMutex", false, false);
	m1.Lock(1000);
	m1.Unlock();

	Boring32::Async::Mutex m2(m1);
	Boring32::Async::Mutex m3(false, false);

	m2 = m3;
	m2 = Boring32::Async::Mutex(L"Assignment", false, false);
}

void TestConversions()
{
	const wchar_t test1[] = L"Test1";
	std::wcout 
		<<	Boring32::Strings::ConvertStringToWString(
				Boring32::Strings::ConvertWStringToString(test1))
		<< std::endl;
}

void TestMemoryMappedFile()
{
	Boring32::Async::MemoryMappedFile m1(L"HelloMmf1", 1000, false);
	Boring32::Async::MemoryMappedFile m2(m1);
	Boring32::Async::MemoryMappedFile m3(L"HelloMmf3", 1000, false);
	m2 = m3;
}

void TestLibraryLoad()
{
	Boring32::Library::DynamicLinkLibrary d(L"wlanapi.dll");
	//void* mainFunc = d.Resolve(L"GetMainInterface");
	//if (mainFunc == nullptr)
	//	throw new std::runtime_error("Could not resolve function");
}

void TestThreadSafeVector()
{
	Boring32::Async::ThreadSafeVector<int> testVector;
	testVector.Add(1);
	testVector.Add(2);
	testVector.Add(1);
	testVector.Add(1);
	testVector.Add(5);
	// Erase all 1s
	testVector.EraseMultiple(
		[](const int& val) -> bool
		{
			return val == 1;
		});
	if(testVector.Size() != 2)
		throw std::runtime_error("Unexpected size of vector");
	if (testVector.CopyOfElementAt(0) != 2)
		throw std::runtime_error("Unexpected element at index 0");
}

void TestAnonPipes()
{
	std::wstring msg1(L"message1");
	std::wstring msg2(L"message2");
	Boring32::Async::AnonymousPipe pipe(true, 512, L"||");
	pipe.DelimitedWrite(msg1);
	pipe.DelimitedWrite(msg2);

	std::vector<std::wstring> response = pipe.DelimitedRead();
	if(response.size() != 2)
		throw std::runtime_error("Unexpected number of tokens");
	std::wcout << response[0] << L" " << response[1] << std::endl;
	if (msg1 != response[0] || msg2 != response[1])
		throw std::runtime_error("Failed to match input to output");
}

void TestProcessBlockingNamedPipe()
{
	std::wstring directory;
	directory.resize(2048);
	GetModuleFileName(nullptr, &directory[0], directory.size());
	PathCchRemoveFileSpec(&directory[0], directory.size());
	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
	std::wstring filePath = directory + L"\\TestProcess.exe";

	Boring32::Async::Job job(false);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{ 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	job.SetInformation(jeli);
	std::wstringstream ss;
	ss	<< "TestProcess.exe"
		<< L" 1";
	Boring32::Async::Process testProcess(filePath, ss.str(), directory, true);
	testProcess.Start();
	job.AssignProcessToThisJob(testProcess.GetProcessHandle());

	Boring32::Async::BlockingNamedPipeServer pipeServer(
		L"\\\\.\\pipe\\mynamedpipe", 
		200, 
		5, 
		L"", 
		false,
		true
	);
	pipeServer.Connect();
	pipeServer.Write(L"HAHA!");
	pipeServer.Write(L"HAHA2!");
	WaitForSingleObject(testProcess.GetProcessHandle(), INFINITE);
	pipeServer.Disconnect();
}

void TestProcessOverlappedNamedPipe()
{
	std::wstring directory;
	directory.resize(2048);
	GetModuleFileName(nullptr, &directory[0], directory.size());
	PathCchRemoveFileSpec(&directory[0], directory.size());
	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
	std::wstring filePath = directory + L"\\TestProcess.exe";

	Boring32::Async::OverlappedNamedPipeServer serverPipe(
		L"\\\\.\\pipe\\mynamedpipe",
		200,
		5,
		L"",
		false,
		true
	);
	auto oio = serverPipe.Connect();

	Boring32::Async::Job job(false);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{ 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	job.SetInformation(jeli);
	std::wstringstream ss;
	ss	<< "TestProcess.exe"
		<< L" 2";
	Boring32::Async::Process testProcess(filePath, ss.str(), directory, true);
	testProcess.Start();
	job.AssignProcessToThisJob(testProcess.GetProcessHandle());

	oio.WaitForCompletion(INFINITE);
	oio = serverPipe.Write(L"HAHA!");
	WaitForSingleObject(testProcess.GetProcessHandle(), INFINITE);
}

void TestProcessAnonPipe()
{
	std::wstring directory;
	directory.resize(2048);
	GetModuleFileName(nullptr, &directory[0], directory.size());
	PathCchRemoveFileSpec(&directory[0], directory.size());
	directory.erase(std::find(directory.begin(), directory.end(), '\0'), directory.end());
	std::wstring filePath = directory + L"\\TestProcess.exe";

	Boring32::Async::Event evt(true, true, false, L"TestEvent");
	Boring32::Async::AnonymousPipe childWrite;
	Boring32::Async::AnonymousPipe childRead;
	childRead = Boring32::Async::AnonymousPipe(true, 2048, L"||");
	childWrite = Boring32::Async::AnonymousPipe(true, 2048, L"||");
	std::wstringstream ss;
	ss << "TestProcess.exe"
		<< L" 3 "
		<< (int)childWrite.GetWrite()
		<< L" "
		<< (int)childRead.GetRead();
	Boring32::Async::Process testProcess(filePath, ss.str(), directory, true);

	Boring32::Async::Job job(false);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{ 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	job.SetInformation(jeli);
	testProcess.Start();
	job.AssignProcessToThisJob(testProcess.GetProcessHandle());

	childRead.DelimitedWrite(L"Hello from parent!");
	Sleep(500);

	std::wcout
		<< childWrite.Read() 
		<< std::endl;
	evt.Signal();
	WaitForSingleObject(testProcess.GetProcessHandle(), INFINITE);
}

void TestCompression()
{
	Boring32::Compression::Compressor compressor(Boring32::Compression::CompressionType::MSZIP);
	Boring32::Compression::Decompressor decompressor(Boring32::Compression::CompressionType::MSZIP);
}

int main(int argc, char** args)
{
	try
	{
		Boring32::Async::OverlappedNamedPipeServer server(L"A", 512, 1, L"", false, true);
		auto a = server.Connect();
		Boring32::Async::OverlappedNamedPipeClient client(L"A");
		client.Connect(0);
		auto b = server.Read(0);
		client.Close();
		b.WaitForCompletion(INFINITE);
		std::wcout << b.IsSuccessful() << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::wcout << ex.what() << std::endl;
	}

	PROCESS_MEMORY_EXHAUSTION_INFO pmei{ 0 };
	pmei.Version = PME_CURRENT_VERSION;
	pmei.Type = PMETypeFailFastOnCommitFailure;
	pmei.Value = PME_FAILFAST_ON_COMMIT_FAIL_ENABLE;
	bool succeeded = SetProcessInformation(
		GetCurrentProcess(),
		ProcessMemoryExhaustionInfo,
		&pmei,
		sizeof(pmei)
	);

	std::wcout << Boring32::Util::GetCurrentExecutableDirectory() << std::endl;

	//TestProcessNamedPipe();
	for (int i = 0; i < 13; i++)
	{
		try
		{
			std::wcout << L"Test: " << i << std::endl;
			if (i == 0)
				TestProcessOverlappedNamedPipe();
			if (i == 1)
				TestThreadSafeVector();
			if (i == 2)
				TestException();
			if (i == 3)
				TestWaitableTime(2000);
			if (i == 4)
				TestSemaphore();
			if (i == 5)
				TestMutex();
			if (i == 6)
				TestConversions();
			if (i == 7)
				TestMemoryMappedFile();
			if (i == 8)
				TestAnonPipes();
			if (i == 9)
				TestLibraryLoad();
			if (i == 10)
				TestProcessBlockingNamedPipe();
			if (i == 11)
				TestProcessAnonPipe();
			if (i == 12)
				TestCompression();
		}
		catch (const std::exception& ex)
		{
			std::wcout << ex.what() << std::endl;
		}
	}
	
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

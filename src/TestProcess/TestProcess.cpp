import std;
import std.compat;
#include <Windows.h>

import boring32.async;
import boring32.ipc;
import boring32.security;
import boring32.util;
import test;

int MainBlocking(int argc, char** args)
{
    Sleep(1000);

    Boring32::IPC::BlockingNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    p.SetMode(PIPE_READMODE_MESSAGE);
    std::wcout << p.ReadAsString() << std::endl;
    std::wcout << p.ReadAsString() << std::endl;
    p.Close();
    return 0;
}

int MainOverlapped(int argc, char** args)
{
    Sleep(1000);
    Boring32::IPC::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    p.SetMode(PIPE_READMODE_MESSAGE);
    Boring32::Async::OverlappedIo readOp;
    p.Read(1024, readOp);
    readOp.WaitForCompletion(INFINITE);
    std::wcout << Boring32::Util::ByteVectorToString<std::wstring>(readOp.IoBuffer) << std::endl;
    Sleep(1000);

    Boring32::Async::OverlappedIo writeOp;
    p.Write(L"Indeed!", writeOp);
    writeOp.WaitForCompletion(INFINITE);
    p.Write(L"Indeed!", writeOp);
    writeOp.WaitForCompletion(INFINITE);
    //p.CloseSocket();;

    return 0;
}

int ConnectAndWriteToElevatedPipe()
{
    Boring32::IPC::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    p.SetMode(PIPE_READMODE_MESSAGE);
    //auto r = p.Write(L"Hello!");
    //WaitForSingleObject(r.IoEvent.GetHandle(), INFINITE);
    return 0;
}

DWORD ConnectToPrivateNamespace()
{
    Boring32::Security::PrivateNamespace pn(
        false,
        false,
        L"elevated",
        L"elevated-boundary",
        L"D:(A;;GA;;;BA)(A;;GR;;;BU)"
    );
    Boring32::Async::Event agentNamespacePipeAvailable(
        true,
        false,
        L"elevated\\pipeAvailable",
        SYNCHRONIZE
    );
    agentNamespacePipeAvailable.WaitOnEvent();
    Boring32::IPC::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\\\mynamedpipe");
    p.Connect(0);
    p.SetMode(PIPE_READMODE_MESSAGE);
    return 0;
}

int main(int argc, char** args)
{
    MyFunc1();
    MyFunc2();
    try
    {
        if (argc < 2)
            throw std::runtime_error("Minimum number of args not given");
        std::string testType(args[1]);
        std::cout << "Child process started in mode: " << testType << std::endl;

        if (testType == "1")
            MainBlocking(argc, args);
        if (testType == "2")
            MainOverlapped(argc, args);

        //return ConnectToPrivateNamespace();
        //return ConnectAndWriteToElevatedPipe();
        std::wcout 
            << L"Child process in mode " 
            << testType.c_str()
            << L" exited without error" 
            << std::endl;
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::wcout << ex.what() << std::endl;
        return 1;
    }
}

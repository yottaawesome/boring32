#include <iostream>
#include <Windows.h>
#include <string>
#include "../Boring32/include/Boring32.hpp"

int MainAnon(int argc, char** args)
{
    if (argc > 0)
    {
        int writeHandle = std::stoi(args[1]);
        int readHandle = std::stoi(args[2]);
        Boring32::Async::AnonymousPipe pipe(2048, L"||", (HANDLE)readHandle, (HANDLE)writeHandle);
        std::wcout << pipe.Read();
        pipe.DelimitedWrite(L"Hello from child!");
    }

    Boring32::Async::Event evt(true, false, L"TestEvent", SYNCHRONIZE);
    evt.WaitOnEvent();
    std::wcout << L"Exiting after wait!" << std::endl;

    return 0;
}

int MainNamed(int argc, char** args)
{
    Sleep(1000);

    Boring32::Async::BlockingNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    std::wcout << p.Read() << std::endl;
    std::wcout << p.Read() << std::endl;
    p.Close();
    return 0;
}

int MainOverlapped(int argc, char** args)
{
    Sleep(1000);

    Boring32::Async::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);

    auto oio = p.Read();
    DWORD bytes = 0;
    oio.GetBytesTransferred(true, bytes);
    if (bytes > 0)
    {
        oio.IoBuffer.resize(bytes / sizeof(wchar_t));
        std::wcout << oio.IoBuffer << std::endl;
    }
    p.Close();

    return 0;
}

int ConnectAndWriteToElevatedPipe()
{
    Boring32::Async::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    //auto r = p.Write(L"Hello!");
    //WaitForSingleObject(r.IoEvent.GetHandle(), INFINITE);
    return 0;
}

DWORD ConnectToPrivateNamespace()
{
    Boring32::Security::PrivateNamespace pn(
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
    Boring32::Async::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\\\mynamedpipe");
    p.Connect(0);
    return 0;
}

int main(int argc, char** args)
{
    try
    {
        return ConnectToPrivateNamespace();
        return ConnectAndWriteToElevatedPipe();
        //MainNamed(argc, args);
        return MainOverlapped(argc, args);
    }
    catch (const std::exception& ex)
    {
        std::wcout << ex.what() << std::endl;
        return 1;
    }
}

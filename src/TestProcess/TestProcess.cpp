#include <iostream>
#include <Windows.h>
#include <string>
#include "../Boring32/include/Boring32.hpp"

int MainAnon(int argc, char** args)
{
    if (argc != 4)
        throw std::runtime_error("MainAnon(): required arguments missing");

    int writeHandle = std::stoi(args[2]);
    int readHandle = std::stoi(args[3]);
    Boring32::Async::AnonymousPipe pipe(2048, L"||", (HANDLE)readHandle, (HANDLE)writeHandle);
    std::wcout << pipe.Read() << std::endl;
    pipe.DelimitedWrite(L"Hello from child!");

    Boring32::Async::Event evt(false, false, L"TestEvent", SYNCHRONIZE);
    evt.WaitOnEvent();
    std::wcout << L"Exiting after wait!" << std::endl;

    return 0;
}

int MainBlocking(int argc, char** args)
{
    Sleep(1000);

    Boring32::Async::BlockingNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
    p.Connect(0);
    p.SetMode(PIPE_READMODE_MESSAGE);
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
    p.SetMode(PIPE_READMODE_MESSAGE);
    auto oio = p.Read();
    oio.WaitForCompletion(INFINITE);
    oio.ResizeBuffer();
    std::wcout << oio.IoBuffer << std::endl;
    p.Close();

    return 0;
}

int ConnectAndWriteToElevatedPipe()
{
    Boring32::Async::OverlappedNamedPipeClient p(L"\\\\.\\pipe\\mynamedpipe");
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
    p.SetMode(PIPE_READMODE_MESSAGE);
    return 0;
}

int main(int argc, char** args)
{
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
        if (testType == "3")
            MainAnon(argc, args);

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

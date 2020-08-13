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

    Boring32::Async::Event evt(false, true, true, false, L"TestEvent");
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

    std::wstring buffer;
    auto oio = p.Read(buffer);
    WaitForSingleObject(oio.IoEvent.GetHandle(), INFINITE);

    std::wcout << buffer << std::endl;
    p.Close();
    return 0;
}

int main(int argc, char** args)
{
    try
    {
        //MainNamed(argc, args);
        MainOverlapped(argc, args);
    }
    catch (const std::exception& ex)
    {
        std::wcout << ex.what() << std::endl;
        return 1;
    }
}

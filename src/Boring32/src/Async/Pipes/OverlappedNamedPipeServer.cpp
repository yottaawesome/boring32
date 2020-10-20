#include "pch.hpp"
#include <stdexcept>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Pipes/OverlappedNamedPipeServer.hpp"

namespace Boring32::Async
{
    OverlappedNamedPipeServer::~OverlappedNamedPipeServer()
    {
        Close();
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer() { }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances,
        const std::wstring& sid,
        const bool isInheritable,
        const bool isLocalPipe
    )
    : NamedPipeServerBase(
        pipeName,
        size,
        maxInstances,
        sid,
        isInheritable,
        PIPE_ACCESS_DUPLEX 
            | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE           // message type pipe 
            | PIPE_READMODE_MESSAGE     // message-read mode
            | PIPE_WAIT
            | (isLocalPipe
                ? PIPE_REJECT_REMOTE_CLIENTS
                : PIPE_ACCEPT_REMOTE_CLIENTS)
    )
    {
        InternalCreatePipe();
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
        const std::wstring& sid,
        const bool isInheritable,
        const DWORD openMode,
        const DWORD pipeMode
    )
    :   NamedPipeServerBase(
            pipeName,
            size,
            maxInstances,
            sid,
            isInheritable,
            openMode,
            pipeMode
        )
    {
        m_openMode |= FILE_FLAG_OVERLAPPED; // Ensure we're overlapped
        InternalCreatePipe();
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(const OverlappedNamedPipeServer& other)
    :   NamedPipeServerBase(other)
    {
        InternalCreatePipe();
    }

    void OverlappedNamedPipeServer::operator=(const OverlappedNamedPipeServer& other)
    {
        Copy(other);
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept
    :   NamedPipeServerBase(std::move(other))
    {
        InternalCreatePipe();
    }

    void OverlappedNamedPipeServer::operator=(OverlappedNamedPipeServer&& other) noexcept
    {
        Move(other);
    }

    void NTAPI Poop(void* data, BYTE timedOut)
    {
        int i = 0;
    }

    bool OverlappedNamedPipeServer::Connect(OverlappedOp& op, std::nothrow_t) noexcept
    {
        try
        {
            op = Connect();
            return true;
        }
        catch (const std::exception& ex)
        {
            std::wcerr << L"OverlappedNamedPipeServer::Connect(OverlappedOp& op, std::nothrow_t): " << ex.what();
            return false;
        }
    }

    OverlappedOp OverlappedNamedPipeServer::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("OverlappedNamedPipeServer::Connect(): No valid pipe handle to connect");
        OverlappedOp oio;
        bool succeeded = ConnectNamedPipe(m_pipe.GetHandle(), oio.GetOverlapped());
        oio.LastError(GetLastError());
        if (succeeded == false && oio.LastError() != ERROR_IO_PENDING)
            throw Error::Win32Error("OverlappedNamedPipeServer::Connect(): ConnectNamedPipe() failed", oio.LastError());
        
        /*
        HANDLE out = nullptr;
        RegisterWaitForSingleObject(
            &out,
            oio.GetWaitableHandle(),
            Poop,//[](void* data, BYTE timedOut)->void {},
            this, 
            INFINITE, 
            WT_EXECUTEDEFAULT
        );
        */

        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Write(const std::wstring& msg)
    {
        return InternalWrite(msg);
    }

    bool OverlappedNamedPipeServer::Write(const std::wstring& msg, OverlappedIo& op) noexcept
    {
        try
        {
            op = InternalWrite(msg);
            return true;
        }
        catch (const std::exception& ex)
        {
            std::wcerr
                << L"OverlappedNamedPipeServer::Write(): "
                << ex.what()
                << std::endl;
            return false;
        }
    }

    OverlappedIo OverlappedNamedPipeServer::InternalWrite(const std::wstring& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("OverlappedNamedPipeServer::InternalWrite(): No pipe to write to");

        OverlappedIo oio;
        bool succeeded = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            (DWORD)(msg.size()*sizeof(wchar_t)), // number of bytes to write 
            nullptr,          // number of bytes written 
            oio.GetOverlapped()       // overlapped I/O
        );
        oio.LastError(GetLastError());
        if (succeeded == false && oio.LastError() != ERROR_IO_PENDING)
            throw Error::Win32Error("OverlappedNamedPipeServer::Write(): WriteFile() failed", oio.LastError());

        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Read(const DWORD noOfCharacters)
    {
        return InternalRead(noOfCharacters);
    }

    bool OverlappedNamedPipeServer::Read(const DWORD noOfCharacters, OverlappedIo& op) noexcept
    {
        try
        {
            op = InternalRead(noOfCharacters);
            return true;
        }
        catch(const std::exception& ex)
        {
            std::wcerr
                << L"OverlappedNamedPipeServer::Read(): "
                << ex.what()
                << std::endl;
            return false;
        }
    }

    OverlappedIo OverlappedNamedPipeServer::InternalRead(const DWORD noOfCharacters)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("OverlappedNamedPipeServer::InternalRead(): No pipe to read from");

        OverlappedIo oio;
        oio.IoBuffer.resize(noOfCharacters);

        bool succeeded = ReadFile(
            m_pipe.GetHandle(),                             // pipe handle 
            &oio.IoBuffer[0],                               // buffer to receive reply 
            (DWORD)(oio.IoBuffer.size()*sizeof(wchar_t)),   // size of buffer, in bytes
            nullptr,                                        // number of bytes read 
            oio.GetOverlapped());                           // overlapped
        oio.LastError(GetLastError());
        if (
            succeeded == false
            && oio.LastError() != ERROR_IO_PENDING
            && oio.LastError() != ERROR_MORE_DATA
        )
        {
            throw Error::Win32Error("OverlappedNamedPipeServer::Read(): ReadFile() failed", oio.LastError());
        }

        return oio;
    }
}

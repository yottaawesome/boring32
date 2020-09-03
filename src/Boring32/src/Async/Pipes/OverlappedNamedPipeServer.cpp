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
        InternalCreatePipe();
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept
    :   NamedPipeServerBase(std::move(other))
    {
        InternalCreatePipe();
    }

    void OverlappedNamedPipeServer::operator=(OverlappedNamedPipeServer&& other) noexcept
    {
        Move(other);
        InternalCreatePipe();
    }

    OverlappedOp OverlappedNamedPipeServer::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to connect");
        OverlappedOp oio(m_pipe);
        oio.CallReturnValue = ConnectNamedPipe(m_pipe.GetHandle(), oio.GetOverlapped());
        oio.LastErrorValue = GetLastError();
        if (oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
            throw Error::Win32Error("OverlappedNamedPipeServer::Connect(): ConnectNamedPipe() failed", oio.LastErrorValue);
        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Write(const std::wstring& msg)
    {
        return InternalWrite(msg, true);
    }

    OverlappedIo OverlappedNamedPipeServer::Write(const std::wstring& msg, const std::nothrow_t)
    {
        return InternalWrite(msg, false);
    }

    OverlappedIo OverlappedNamedPipeServer::InternalWrite(const std::wstring& msg, const bool throwOnWin32Error)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        OverlappedIo oio(m_pipe);
        oio.CallReturnValue = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            msg.size() * sizeof(wchar_t), // number of bytes to write 
            nullptr,          // number of bytes written 
            oio.GetOverlapped()       // overlapped I/O
        );
        oio.LastErrorValue = GetLastError();
        if (throwOnWin32Error && oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
            throw Error::Win32Error("OverlappedNamedPipeServer::Write(): WriteFile() failed", oio.LastErrorValue);

        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Read(const DWORD noOfCharacters)
    {
        return InternalRead(noOfCharacters, true);
    }

    OverlappedIo OverlappedNamedPipeServer::Read(const DWORD noOfCharacters, const std::nothrow_t)
    {
        return InternalRead(noOfCharacters, false);
    }

    OverlappedIo OverlappedNamedPipeServer::InternalRead(const DWORD noOfCharacters, const bool throwOnWin32Error)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        OverlappedIo oio(m_pipe);
        oio.IoBuffer.resize(noOfCharacters);

        oio.CallReturnValue = ReadFile(
            m_pipe.GetHandle(),    // pipe handle 
            &oio.IoBuffer[0],    // buffer to receive reply 
            oio.IoBuffer.size() * sizeof(wchar_t),  // size of buffer 
            nullptr,  // number of bytes read 
            oio.GetOverlapped());    // overlapped
        oio.LastErrorValue = GetLastError();
        if (throwOnWin32Error && oio.LastErrorValue != ERROR_IO_PENDING)
            throw Error::Win32Error("OverlappedNamedPipeServer::Read(): ReadFile() failed", oio.LastErrorValue);

        return oio;
    }
}

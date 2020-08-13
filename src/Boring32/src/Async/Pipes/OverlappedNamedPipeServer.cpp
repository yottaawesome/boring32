#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Pipes/OverlappedNamedPipeServer.hpp"

namespace Boring32::Async
{
    OverlappedNamedPipeServer::~OverlappedNamedPipeServer()
    {
        Close();
    }

    OverlappedNamedPipeServer::OverlappedNamedPipeServer(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances,
        const bool isLocalPipe
    )
    : NamedPipeServerBase(
        pipeName,
        size,
        maxInstances,
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
        const DWORD openMode,
        const DWORD pipeMode
    )
    :   NamedPipeServerBase(
            pipeName,
            size,
            maxInstances,
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
    :   NamedPipeServerBase(other)
    {
        InternalCreatePipe();
    }

    void OverlappedNamedPipeServer::operator=(OverlappedNamedPipeServer&& other) noexcept
    {
        Move(other);
        InternalCreatePipe();
    }

    OverlappedIo OverlappedNamedPipeServer::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to connect");
        OverlappedIo oio;
        oio.IoHandle = m_pipe;
        oio.CallReturnValue = ConnectNamedPipe(m_pipe.GetHandle(), &oio.IoOverlapped);
        oio.LastErrorValue = GetLastError();
        if(oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
            throw std::runtime_error("Failed to connect named pipe");
        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Write(const std::wstring& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        OverlappedIo oio;
        oio.IoHandle = m_pipe;
        DWORD bytesWritten = 0;
        oio.CallReturnValue = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            msg.size() * sizeof(wchar_t), // number of bytes to write 
            &bytesWritten,          // number of bytes written 
            &oio.IoOverlapped       // overlapped I/O
        );
        oio.LastErrorValue = GetLastError();
        if (oio.CallReturnValue == false && oio.LastErrorValue != ERROR_IO_PENDING)
            throw std::runtime_error("Failed to read pipe");

        return oio;
    }

    OverlappedIo OverlappedNamedPipeServer::Read(std::wstring& dataBuffer)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        constexpr DWORD blockSize = 1024;
        dataBuffer.resize(blockSize);

        OverlappedIo oio;
        oio.IoHandle = m_pipe;
        DWORD totalBytesRead = 0;
        bool continueReading = true;
        while (continueReading)
        {
            DWORD currentBytesRead = 0;
            oio.CallReturnValue = ReadFile(
                m_pipe.GetHandle(),    // pipe handle 
                &dataBuffer[0],    // buffer to receive reply 
                dataBuffer.size() * sizeof(TCHAR),  // size of buffer 
                &currentBytesRead,  // number of bytes read 
                &oio.IoOverlapped);    // overlapped
            totalBytesRead += currentBytesRead;
            oio.LastErrorValue = GetLastError();
            if (oio.LastErrorValue == ERROR_MORE_DATA)
                dataBuffer.resize(dataBuffer.size() + blockSize);
            else if (oio.LastErrorValue != ERROR_IO_PENDING)
                throw std::runtime_error("Failed to read from pipe");
        }

        if (totalBytesRead > 0)
            dataBuffer.resize(totalBytesRead / sizeof(wchar_t));

        return oio;
    }
}

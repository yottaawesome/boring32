#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Pipes/BlockingNamedPipeServer.hpp"

namespace Boring32::Async
{
	BlockingNamedPipeServer::~BlockingNamedPipeServer()
	{
        Close();
    }

	BlockingNamedPipeServer::BlockingNamedPipeServer(
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
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE           // message type pipe 
            | PIPE_READMODE_MESSAGE     // message-read mode
            | PIPE_WAIT
            | (isLocalPipe 
                ? PIPE_REJECT_REMOTE_CLIENTS 
                : PIPE_ACCEPT_REMOTE_CLIENTS)
    )
	{
        m_openMode &= ~FILE_FLAG_OVERLAPPED;
        InternalCreatePipe();
	}

    BlockingNamedPipeServer::BlockingNamedPipeServer(
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
        m_openMode &= ~FILE_FLAG_OVERLAPPED; // Negate overlapped flag
        InternalCreatePipe();
    }

    BlockingNamedPipeServer::BlockingNamedPipeServer(const BlockingNamedPipeServer& other)
    :   NamedPipeServerBase(other)
    {
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::operator=(const BlockingNamedPipeServer& other)
    {
        Copy(other);
    }

    BlockingNamedPipeServer::BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept
    :   NamedPipeServerBase(std::move(other))
    {
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::operator=(BlockingNamedPipeServer&& other) noexcept
    {
        Move(other);
    }

    void BlockingNamedPipeServer::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to connect");

        if (ConnectNamedPipe(m_pipe.GetHandle(), nullptr) == false)
            throw std::runtime_error("Failed to connect named pipe");
    }

    void BlockingNamedPipeServer::Write(const std::wstring& msg)
    {
        InternalWrite(msg);
    }

    bool BlockingNamedPipeServer::Write(const std::wstring& msg, const std::nothrow_t)
    {
        try
        {
            InternalWrite(msg);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void BlockingNamedPipeServer::InternalWrite(const std::wstring& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        DWORD bytesWritten = 0;
        bool success = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            (DWORD)(msg.size()*sizeof(wchar_t)), // number of bytes to write 
            &bytesWritten,          // number of bytes written 
            nullptr                 // not overlapped I/O
        );               
        if (success == false)
            throw std::runtime_error("Failed to read pipe");
    }

    std::wstring BlockingNamedPipeServer::Read()
    {
        return InternalRead();
    }

    bool BlockingNamedPipeServer::Read(std::wstring& out, const std::nothrow_t)
    {
        try
        {
            out = InternalRead();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::wstring BlockingNamedPipeServer::InternalRead()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        std::wstring dataBuffer;
        constexpr DWORD blockSize = 1024;
        dataBuffer.resize(1024);

        bool continueReading = true;
        DWORD totalBytesRead = 0;
        while (continueReading)
        {
            DWORD currentBytesRead = 0;
            // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
            bool successfulRead = ReadFile(
                m_pipe.GetHandle(),    // pipe handle 
                &dataBuffer[0],    // buffer to receive reply 
                (DWORD)(dataBuffer.size() * sizeof(TCHAR)),  // size of buffer 
                &currentBytesRead,  // number of bytes read 
                nullptr);    // not overlapped
            totalBytesRead += currentBytesRead;

            const DWORD lastError = GetLastError();
            if (successfulRead == false && lastError != ERROR_MORE_DATA)
                throw std::runtime_error("Failed to read from pipe");
            if (lastError == ERROR_MORE_DATA)
                dataBuffer.resize(dataBuffer.size() + blockSize);
            continueReading = !successfulRead;
        }

        if (totalBytesRead > 0)
            dataBuffer.resize(totalBytesRead / sizeof(wchar_t));

        return dataBuffer;
    }
}
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
        const bool isLocalPipe
    )
    : NamedPipeServerBase(
        pipeName, 
        size, 
        maxInstances, 
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE           // message type pipe 
        | PIPE_READMODE_MESSAGE     // message-read mode
        | PIPE_WAIT
        | (isLocalPipe 
            ? PIPE_REJECT_REMOTE_CLIENTS 
            : PIPE_ACCEPT_REMOTE_CLIENTS)
    )
	{
        if (isLocalPipe)
            m_pipeMode |= PIPE_ACCEPT_REMOTE_CLIENTS;
        else
            m_pipeMode |= PIPE_REJECT_REMOTE_CLIENTS;
        m_openMode &= ~FILE_FLAG_OVERLAPPED;
        InternalCreatePipe();
	}

    BlockingNamedPipeServer::BlockingNamedPipeServer(
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
        m_openMode &= ~FILE_FLAG_OVERLAPPED; // Negate overlapped flag
        InternalCreatePipe();
    }

    BlockingNamedPipeServer::BlockingNamedPipeServer(const BlockingNamedPipeServer& other)
        : NamedPipeServerBase(
            other.m_pipeName,
            other.m_size,
            other.m_maxInstances,
            other.m_openMode,
            other.m_pipeMode)
    {
        Copy(other);
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::operator=(const BlockingNamedPipeServer& other)
    {
        Copy(other);
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::Copy(const BlockingNamedPipeServer& other)
    {
        Close();
        m_pipe = other.m_pipe;
        m_pipeName = other.m_pipeName;
        m_size = other.m_size;
        m_maxInstances = other.m_maxInstances;
        m_isConnected = other.m_isConnected;
        m_openMode = other.m_openMode;
        m_pipeMode = other.m_pipeMode;
    }

    BlockingNamedPipeServer::BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept
    : NamedPipeServerBase(
        other.m_pipeName,
        other.m_size,
        other.m_maxInstances,
        other.m_openMode,
        other.m_pipeMode)
    {
        Move(other);
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::operator=(BlockingNamedPipeServer&& other) noexcept
    {
        Move(other);
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::Move(BlockingNamedPipeServer& other) noexcept
    {
        Close();
        m_pipeName = std::move(other.m_pipeName);
        m_size = other.m_size;
        m_maxInstances = other.m_maxInstances;
        m_isConnected = other.m_isConnected;
        m_openMode = other.m_openMode;
        m_pipeMode = other.m_pipeMode;
        if (other.m_pipe != nullptr)
            m_pipe = std::move(other.m_pipe);
    }

    void BlockingNamedPipeServer::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to connect");

        if (ConnectNamedPipe(m_pipe.GetHandle(), nullptr) == false)
            throw std::runtime_error("Failed to connect named pipe");
    }

    void BlockingNamedPipeServer::Disconnect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to disconnect");
        if (m_isConnected)
        {
            DisconnectNamedPipe(m_pipe.GetHandle());
            m_isConnected = false;
        }
    }

    void BlockingNamedPipeServer::Write(const std::wstring& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        DWORD bytesWritten = 0;
        bool success = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            msg.size() * sizeof(wchar_t), // number of bytes to write 
            &bytesWritten,          // number of bytes written 
            nullptr                 // not overlapped I/O
        );               
        if (success == false)
            throw std::runtime_error("Failed to read pipe");
    }

    std::wstring BlockingNamedPipeServer::Read()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        std::wstring data;
        data.resize(m_size * sizeof(wchar_t));
        DWORD bytesRead = 0;
        bool success = ReadFile(
            m_pipe.GetHandle(),             // handle to pipe 
            &data[0],                       // buffer to receive data 
            data.size() * sizeof(wchar_t),    // size of buffer 
            &bytesRead,                     // number of bytes read 
            nullptr                         // not overlapped I/O
        );
        if (success == false)
            throw std::runtime_error("Failed to read pipe");

        return data;
    }
}
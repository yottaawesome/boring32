#include "pch.hpp"
#include <stdexcept>
#include "include/Async/BlockingNamedPipeServer.hpp"

namespace Boring32::Async
{
	BlockingNamedPipeServer::~BlockingNamedPipeServer()
	{
        Close();
    }
    
    void BlockingNamedPipeServer::Close()
    {
        m_pipe.Close();
    }

	BlockingNamedPipeServer::BlockingNamedPipeServer(
        const std::wstring& pipeName, 
        const DWORD size,
        const DWORD maxInstances,
        const bool isLocalPipe
    )
    :   m_pipeName(pipeName),
        m_size(size),
        m_maxInstances(maxInstances),
        m_isConnected(false),
        m_openMode(
            PIPE_ACCESS_DUPLEX          // read/write access
        ),
        m_pipeMode(
            PIPE_TYPE_MESSAGE           // message type pipe 
            | PIPE_READMODE_MESSAGE     // message-read mode
            | PIPE_WAIT
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
        : m_pipeName(pipeName),
        m_size(size),
        m_maxInstances(maxInstances),
        m_isConnected(false),
        m_openMode(openMode),
        m_pipeMode(pipeMode)
    {
        m_openMode &= ~FILE_FLAG_OVERLAPPED; // Negate overlapped flag
        InternalCreatePipe();
    }

    void BlockingNamedPipeServer::InternalCreatePipe()
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
        m_pipe = CreateNamedPipeW(
            m_pipeName.c_str(),             // pipe name
            m_openMode,
            m_pipeMode,
            m_maxInstances,                 // max. instances  
            m_size,                         // output buffer size 
            m_size,                         // input buffer size 
            0,                              // client time-out 
            nullptr);
        if (m_pipe == nullptr)
            throw std::runtime_error("Failed to create named pipe");
    }

    BlockingNamedPipeServer::BlockingNamedPipeServer(const BlockingNamedPipeServer& other)
    :   m_size(0),
        m_isConnected(false)
    {
        Copy(other);
    }

    void BlockingNamedPipeServer::operator=(const BlockingNamedPipeServer& other)
    {
        Copy(other);
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
    :   m_size(0)
    {
        Move(other);
    }

    void BlockingNamedPipeServer::operator=(BlockingNamedPipeServer&& other) noexcept
    {
        Move(other);
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

    Raii::Win32Handle& BlockingNamedPipeServer::GetInternalHandle()
    {
        return m_pipe;
    }

    std::wstring BlockingNamedPipeServer::GetName() const
    {
        return m_pipeName;
    }

    DWORD BlockingNamedPipeServer::GetSize() const
    {
        return m_size;
    }

    DWORD BlockingNamedPipeServer::GetMaxInstances() const
    {
        return m_maxInstances;
    }

    bool BlockingNamedPipeServer::IsConnected() const
    {
        return m_isConnected;
    }

    DWORD BlockingNamedPipeServer::GetPipeMode() const
    {
        return m_pipeMode;
    }

    DWORD BlockingNamedPipeServer::GetOpenMode() const
    {
        return m_openMode;
    }
}
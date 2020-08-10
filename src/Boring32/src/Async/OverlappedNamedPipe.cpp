#include "pch.hpp"
#include <stdexcept>
#include "include/Async/OverlappedNamedPipe.hpp"

namespace Boring32::Async
{
    OverlappedNamedPipe::~OverlappedNamedPipe()
    {
        Close();
    }

    void OverlappedNamedPipe::Close()
    {
        m_pipe.Close();
    }

    OverlappedNamedPipe::OverlappedNamedPipe(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances
    )
        : m_pipeName(pipeName),
        m_size(size),
        m_maxInstances(maxInstances),
        m_isConnected(false)
    {
        m_pipe = CreateNamedPipeW(
            m_pipeName.c_str(),             // pipe name
            PIPE_ACCESS_DUPLEX              // read/write access
                | FILE_FLAG_OVERLAPPED,              
            PIPE_TYPE_MESSAGE               // message type pipe 
                | PIPE_READMODE_MESSAGE     // message-read mode
                | PIPE_WAIT,                // blocking mode 
            m_maxInstances,                 // max. instances  
            m_size,                         // output buffer size 
            m_size,                         // input buffer size 
            0,                              // client time-out 
            nullptr);
        if (m_pipe == nullptr)
            throw std::runtime_error("Failed to create named pipe");
    }

    OverlappedNamedPipe::OverlappedNamedPipe(const OverlappedNamedPipe& other)
        : m_size(0),
        m_isConnected(false)
    {
        Copy(other);
    }

    void OverlappedNamedPipe::operator=(const OverlappedNamedPipe& other)
    {
        Copy(other);
    }

    void OverlappedNamedPipe::Copy(const OverlappedNamedPipe& other)
    {
        Close();
        m_pipe = other.m_pipe;
        m_pipeName = other.m_pipeName;
        m_size = other.m_size;
        m_maxInstances = other.m_maxInstances;
        m_isConnected = other.m_isConnected;
    }

    OverlappedNamedPipe::OverlappedNamedPipe(OverlappedNamedPipe&& other) noexcept
        : m_size(0)
    {
        Move(other);
    }

    void OverlappedNamedPipe::operator=(OverlappedNamedPipe&& other) noexcept
    {
        Move(other);
    }

    void OverlappedNamedPipe::Move(OverlappedNamedPipe& other) noexcept
    {
        Close();
        m_pipeName = std::move(other.m_pipeName);
        m_size = other.m_size;
        m_maxInstances = other.m_maxInstances;
        m_isConnected = other.m_isConnected;
        if (other.m_pipe != nullptr)
            m_pipe = std::move(other.m_pipe);
    }

    OverlappedIo OverlappedNamedPipe::Connect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to connect");
        OverlappedIo oio;
        if (ConnectNamedPipe(m_pipe.GetHandle(), &oio.IoOverlapped) == false)
            throw std::runtime_error("Failed to connect named pipe");
        return oio;
    }

    void OverlappedNamedPipe::Disconnect()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No valid pipe handle to disconnect");
        if (m_isConnected)
        {
            DisconnectNamedPipe(m_pipe.GetHandle());
            m_isConnected = false;
        }
    }

    OverlappedIo OverlappedNamedPipe::Write(const std::wstring& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        OverlappedIo oio;
        DWORD bytesWritten = 0;
        bool success = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            msg.size() * sizeof(wchar_t), // number of bytes to write 
            &bytesWritten,          // number of bytes written 
            &oio.IoOverlapped       // overlapped I/O
        );
        if (success == false)
            throw std::runtime_error("Failed to read pipe");

        return oio;
    }

    OverlappedIo OverlappedNamedPipe::Read(std::wstring& data)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        data.resize(m_size * sizeof(wchar_t));
        OverlappedIo oio;
        DWORD bytesRead = 0;
        bool success = ReadFile(
            m_pipe.GetHandle(),             // handle to pipe 
            &data[0],                       // buffer to receive data 
            data.size() * sizeof(wchar_t),  // size of buffer 
            &bytesRead,                     // number of bytes read 
            &oio.IoOverlapped               // overlapped I/O
        );
        if (success == false)
            throw std::runtime_error("Failed to read pipe");

        return oio;
    }

    Raii::Win32Handle& OverlappedNamedPipe::GetInternalHandle()
    {
        return m_pipe;
    }

    std::wstring OverlappedNamedPipe::GetName() const
    {
        return m_pipeName;
    }

    DWORD OverlappedNamedPipe::GetSize() const
    {
        return m_size;
    }

    DWORD OverlappedNamedPipe::GetMaxInstances() const
    {
        return m_maxInstances;
    }

    bool OverlappedNamedPipe::IsConnected() const
    {
        return m_isConnected;
    }
}
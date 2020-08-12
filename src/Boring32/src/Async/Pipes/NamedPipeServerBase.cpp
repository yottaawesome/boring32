#include "pch.hpp"
#include "include/Async/Pipes/NamedPipeServerBase.hpp"

namespace Boring32::Async
{
	NamedPipeServerBase::~NamedPipeServerBase()
	{
		Close();
	}

    void NamedPipeServerBase::Close()
    {
        m_pipe.Close();
    }

    NamedPipeServerBase::NamedPipeServerBase(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances,
        const bool isLocalPipe
    )
        : m_pipeName(pipeName),
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
    }

    NamedPipeServerBase::NamedPipeServerBase(
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
    }

    void NamedPipeServerBase::InternalCreatePipe()
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

    NamedPipeServerBase::NamedPipeServerBase(const NamedPipeServerBase& other)
        : m_size(0),
        m_isConnected(false)
    {
        Copy(other);
    }

    void NamedPipeServerBase::operator=(const NamedPipeServerBase& other)
    {
        Copy(other);
    }

    void NamedPipeServerBase::Copy(const NamedPipeServerBase& other)
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

    NamedPipeServerBase::NamedPipeServerBase(NamedPipeServerBase&& other) noexcept
        : m_size(0)
    {
        Move(other);
    }

    void NamedPipeServerBase::operator=(NamedPipeServerBase&& other) noexcept
    {
        Move(other);
    }

    void NamedPipeServerBase::Move(NamedPipeServerBase& other) noexcept
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

    Raii::Win32Handle& NamedPipeServerBase::GetInternalHandle()
    {
        return m_pipe;
    }

    std::wstring NamedPipeServerBase::GetName() const
    {
        return m_pipeName;
    }

    DWORD NamedPipeServerBase::GetSize() const
    {
        return m_size;
    }

    DWORD NamedPipeServerBase::GetMaxInstances() const
    {
        return m_maxInstances;
    }

    bool NamedPipeServerBase::IsConnected() const
    {
        return m_isConnected;
    }

    DWORD NamedPipeServerBase::GetPipeMode() const
    {
        return m_pipeMode;
    }

    DWORD NamedPipeServerBase::GetOpenMode() const
    {
        return m_openMode;
    }
}
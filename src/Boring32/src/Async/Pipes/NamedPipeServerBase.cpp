#include "pch.hpp"
#include <Sddl.h>
#include "include/Error/Win32Error.hpp"
#include "include/Async/Pipes/NamedPipeServerBase.hpp"

namespace Boring32::Async
{
	NamedPipeServerBase::~NamedPipeServerBase()
	{
		Close();
	}

    void NamedPipeServerBase::Close()
    {
        Disconnect();
        m_pipe.Close();
    }

    NamedPipeServerBase::NamedPipeServerBase()
    :   m_pipeName(L""),
        m_size(0),
        m_maxInstances(0),
        m_sid(L""),
        m_isInheritable(false),
        m_openMode(0),
        m_pipeMode(0)
    { }

    NamedPipeServerBase::NamedPipeServerBase(
        const std::wstring& pipeName,
        const DWORD size,
        const DWORD maxInstances,
        const std::wstring& sid,
        const bool isInheritable,
        const bool isLocalPipe
    )
        : m_pipeName(pipeName),
        m_size(size),
        m_maxInstances(maxInstances),
        m_sid(sid),
        m_isInheritable(isInheritable),
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
        const std::wstring& sid,
        const bool isInheritable,
        const DWORD openMode,
        const DWORD pipeMode
    )
        : m_pipeName(pipeName),
        m_size(size),
        m_maxInstances(maxInstances),
        m_sid(sid),
        m_isInheritable(isInheritable),
        m_openMode(openMode),
        m_pipeMode(pipeMode)
    {
    }

    void NamedPipeServerBase::InternalCreatePipe()
    {
        if (m_pipeName.starts_with(L"\\\\.\\pipe\\") == false)
            m_pipeName = L"\\\\.\\pipe\\" + m_pipeName;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = m_isInheritable;
        if (m_sid != L"")
        {
            bool converted = ConvertStringSecurityDescriptorToSecurityDescriptorW(
                m_sid.c_str(),
                SDDL_REVISION_1,
                &sa.lpSecurityDescriptor,
                nullptr
            );
            if (converted == false)
                throw Error::Win32Error("Failed to convert security descriptor", GetLastError());
        }

        // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
        m_pipe = CreateNamedPipeW(
            m_pipeName.c_str(),             // pipe name
            m_openMode,
            m_pipeMode,
            m_maxInstances,                 // max. instances  
            m_size,                         // output buffer size 
            m_size,                         // input buffer size 
            0,                              // client time-out 
            m_sid != L"" ? &sa : nullptr);
        if (m_sid != L"")
            LocalFree(sa.lpSecurityDescriptor);
        if (m_pipe == nullptr)
            throw Error::Win32Error("Failed to create named pipe", GetLastError());
    }

    NamedPipeServerBase::NamedPipeServerBase(const NamedPipeServerBase& other)
        : m_size(0)
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
        m_sid = other.m_sid;
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
        m_sid = std::move(other.m_sid);
        m_openMode = other.m_openMode;
        m_pipeMode = other.m_pipeMode;
        if (other.m_pipe != nullptr)
            m_pipe = std::move(other.m_pipe);
    }

    void NamedPipeServerBase::Disconnect()
    {
        if (m_pipe != nullptr)
            DisconnectNamedPipe(m_pipe.GetHandle());
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

    DWORD NamedPipeServerBase::GetPipeMode() const
    {
        return m_pipeMode;
    }

    DWORD NamedPipeServerBase::GetOpenMode() const
    {
        return m_openMode;
    }

    DWORD NamedPipeServerBase::UnreadCharactersRemaining() const
    {
        DWORD charactersRemaining = 0;
        InternalUnreadCharactersRemaining(charactersRemaining, true);
        return charactersRemaining;
    }

    bool NamedPipeServerBase::UnreadCharactersRemaining(DWORD& charactersRemaining, std::nothrow_t) const noexcept
    {
        return InternalUnreadCharactersRemaining(charactersRemaining, false);
    }

    bool NamedPipeServerBase::InternalUnreadCharactersRemaining(DWORD& charactersRemaining, const bool throwOnError) const
    {
        if (m_pipe == nullptr)
            return false;
        charactersRemaining = 0;
        bool succeeded = PeekNamedPipe(
            m_pipe.GetHandle(),
            nullptr,
            0,
            nullptr,
            nullptr,
            &charactersRemaining
        );
        if (succeeded == false)
        {
            if (throwOnError)
                throw Error::Win32Error("NamedPipeServerBase::InternalUnreadCharactersRemaining(): PeekNamedPipe() failed", GetLastError());
            return false;
        }

        charactersRemaining /= sizeof(wchar_t);
        return true;
    }

    void NamedPipeServerBase::Flush()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to flush");
        if (FlushFileBuffers(m_pipe.GetHandle()) == false)
            throw Error::Win32Error("NamedPipeClientBase::Flush() failed", GetLastError());
    }

    void NamedPipeServerBase::CancelCurrentThreadIo()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("NamedPipeServerBase::CancelCurrentThreadIo(): pipe is nullptr");
        if (CancelIo(m_pipe.GetHandle()) == false)
            throw Error::Win32Error("NamedPipeServerBase::CancelCurrentThreadIo(): CancelIo failed", GetLastError());
    }

    bool NamedPipeServerBase::CancelCurrentThreadIo(std::nothrow_t) noexcept
    {
        try
        {
            CancelCurrentThreadIo();
            return true;
        }
        catch (const std::exception& ex)
        {
            std::wcerr << L"NamedPipeServerBase::CancelCurrentThreadIo(std::nothrow_t) failed: " << ex.what() << std::endl;
            return false;
        }
    }

    void NamedPipeServerBase::CancelCurrentProcessIo(OVERLAPPED* overlapped)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("NamedPipeServerBase::CancelCurrentProcessIo(): pipe is nullptr");
        if (CancelIoEx(m_pipe.GetHandle(), overlapped) == false)
            throw Error::Win32Error("NamedPipeServerBase::CancelCurrentThreadIo(): CancelIo failed", GetLastError());
    }

    bool NamedPipeServerBase::CancelCurrentProcessIo(OVERLAPPED* overlapped, std::nothrow_t) noexcept
    {
        try
        {
            CancelCurrentProcessIo(overlapped);
            return true;
        }
        catch (const std::exception& ex)
        {
            std::wcerr << L"NamedPipeServerBase::CancelCurrentProcessIo(OVERLAPPED*, std::nothrow_t) failed: " << ex.what() << std::endl;
            return false;
        }
    }
}
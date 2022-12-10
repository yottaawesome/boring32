module boring32.ipc:blockingnamedpipeserver;
import boring32.util;
import <string>;
import <stdexcept>;

namespace Boring32::IPC
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
        InternalWrite(Util::StringToByteVector(msg));
    }

    bool BlockingNamedPipeServer::Write(
        const std::wstring& msg, 
        const std::nothrow_t&
    ) noexcept try
    {
        InternalWrite(Util::StringToByteVector(msg));
        return true;
    }
    catch (...)
    {
        return false;
    }

    void BlockingNamedPipeServer::InternalWrite(const std::vector<std::byte>& msg)
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to write to");

        DWORD bytesWritten = 0;
        bool success = WriteFile(
            m_pipe.GetHandle(),     // handle to pipe 
            &msg[0],                // buffer to write from 
            (DWORD)msg.size(), // number of bytes to write 
            &bytesWritten,          // number of bytes written 
            nullptr                 // not overlapped I/O
        );               
        if (success == false)
            throw std::runtime_error("Failed to read pipe");
    }

    std::wstring BlockingNamedPipeServer::ReadAsString()
    {
        return Util::ByteVectorToString<std::wstring>(InternalRead());
    }

    bool BlockingNamedPipeServer::ReadAsString(
        std::wstring& out, 
        const std::nothrow_t&
    ) noexcept
    try
    {
        out = Util::ByteVectorToString<std::wstring>(InternalRead());
        return true;
    }
    catch (...)
    {
        return false;
    }

    std::vector<std::byte> BlockingNamedPipeServer::InternalRead()
    {
        if (m_pipe == nullptr)
            throw std::runtime_error("No pipe to read from");

        constexpr DWORD blockSize = 1024;
        std::vector<std::byte> dataBuffer(blockSize);

        bool continueReading = true;
        DWORD totalBytesRead = 0;
        while (continueReading)
        {
            DWORD currentBytesRead = 0;
            // https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
            bool successfulRead = ReadFile(
                m_pipe.GetHandle(),    // pipe handle 
                &dataBuffer[0],    // buffer to receive reply 
                (DWORD)dataBuffer.size(),  // size of buffer 
                &currentBytesRead,  // number of bytes read 
                nullptr // not overlapped
            );    
            totalBytesRead += currentBytesRead;

            const DWORD lastError = GetLastError();
            if (successfulRead == false && lastError != ERROR_MORE_DATA)
                throw std::runtime_error("Failed to read from pipe");
            if (lastError == ERROR_MORE_DATA)
                dataBuffer.resize(dataBuffer.size() + blockSize);
            continueReading = !successfulRead;
        }

        if (totalBytesRead > 0)
            dataBuffer.resize(totalBytesRead);

        return dataBuffer;
    }
}
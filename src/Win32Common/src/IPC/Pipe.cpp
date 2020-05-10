#include "pch.hpp"
#include <stdexcept>
#include "include/Win32Utils.hpp"

namespace Win32Utils::IPC
{
	Pipe::~Pipe()
	{
		Cleanup();
	}

	Pipe::Pipe()
	:	m_size(0),
		m_inheritable(false),
		m_readHandle(nullptr),
		m_writeHandle(nullptr)
	{ }

	Pipe::Pipe(const Pipe& other)
	:	m_size(other.m_size),
		m_inheritable(other.m_inheritable),
		m_delimiter(other.m_delimiter),
		m_readHandle(nullptr),
		m_writeHandle(nullptr)
	{
		Duplicate(other);
	}

	Pipe::Pipe(Pipe&& other)
	:	m_size(0),
		m_inheritable(false),
		m_readHandle(nullptr),
		m_writeHandle(nullptr)
	{
		m_size = other.m_size;
		m_inheritable = other.m_inheritable;
		m_readHandle = other.m_readHandle;
		m_writeHandle = other.m_writeHandle;
		other.m_size = 0;
		other.m_inheritable = false;
		other.m_readHandle = nullptr;
		other.m_writeHandle = nullptr;
	}

	void Pipe::operator=(Pipe&& other) noexcept
	{
		Cleanup();
		m_size = other.m_size;
		m_delimiter = other.m_delimiter;
		m_inheritable = other.m_inheritable;
		if (other.m_readHandle)
		{
			m_readHandle = other.m_readHandle;
			other.m_readHandle = nullptr;
		}
		if (other.m_writeHandle)
		{
			m_writeHandle = other.m_writeHandle;
			other.m_writeHandle = nullptr;
		}
	}

	Pipe::Pipe(const bool inheritable, const DWORD size, const std::wstring& delimiter)
	:	m_readHandle(nullptr),
		m_writeHandle(nullptr),
		m_inheritable(inheritable),
		m_size(size),
		m_delimiter(delimiter)
	{
		SECURITY_ATTRIBUTES lp{ 0 };
		lp.nLength = sizeof(lp);
		lp.bInheritHandle = m_inheritable;
		bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, &lp, size);
		//DWORD mode = PIPE_READMODE_MESSAGE;
		//SetNamedPipeHandleState(m_readHandle, &mode, nullptr, nullptr);
		//SetNamedPipeHandleState(m_writeHandle, &mode, nullptr, nullptr);
		if (succeeded == false)
			throw std::runtime_error("Failed to create anonymous pipe");
	}

	Pipe::Pipe(
		const bool inheritable,
		const DWORD size,
		const bool duplicate,
		const std::wstring& delimiter,
		const HANDLE readHandle,
		const HANDLE writeHandle
	)
	:	m_readHandle(duplicate ? nullptr : readHandle),
		m_writeHandle(duplicate ? nullptr : writeHandle),
		m_inheritable(inheritable),
		m_delimiter(delimiter),
		m_size(size)
	{
		if(duplicate)
			Duplicate(readHandle, writeHandle);
	}

	void Pipe::operator=(const Pipe& other)
	{
		Cleanup();
		Duplicate(other);
	}

	void Pipe::SetPipes(
		const bool inheritable,
		const DWORD size,
		const bool duplicate,
		const std::wstring& delimiter,
		const HANDLE readHandle,
		const HANDLE writeHandle)
	{
		Cleanup();
		m_size = size;
		m_inheritable = inheritable;
		m_delimiter = delimiter;
		if (duplicate)
		{
			Duplicate(readHandle, writeHandle);
		}
		else
		{
			m_readHandle = readHandle;
			m_writeHandle = writeHandle;
		}
	}

	void Pipe::Duplicate(const HANDLE readHandle, const HANDLE writeHandle)
	{
		if (readHandle)
		{
			bool succeeded = DuplicateHandle(
				GetCurrentProcess(),
				readHandle,
				GetCurrentProcess(),
				&m_readHandle,
				0,
				m_inheritable,
				DUPLICATE_SAME_ACCESS
			);
			if (succeeded == false)
				throw std::runtime_error("Failed to duplicate handle.");
		}
		if (writeHandle)
		{
			bool succeeded = DuplicateHandle(
				GetCurrentProcess(),
				writeHandle,
				GetCurrentProcess(),
				&m_writeHandle,
				0,
				m_inheritable,
				DUPLICATE_SAME_ACCESS
			);
			if (succeeded == false)
				throw std::runtime_error("Failed to duplicate handle.");
		}
	}

	void Pipe::Duplicate(const Pipe& other)
	{
		m_size = other.m_size;
		m_inheritable = other.m_inheritable;
		m_readHandle = nullptr;
		m_writeHandle = nullptr;

		Duplicate(other.m_readHandle, other.m_writeHandle);
	}

	void Pipe::Cleanup()
	{
		if (m_readHandle)
		{
			CloseHandle(m_readHandle);
			m_readHandle = nullptr;
		}
		if (m_writeHandle)
		{
			CloseHandle(m_writeHandle);
			m_writeHandle = nullptr;
		}
	}

	void Pipe::Write(const std::wstring& msg)
	{
		if (m_writeHandle == nullptr)
			throw std::runtime_error("No active write handle.");

		std::wstring msg2(msg);
		if (m_delimiter != L"")
			msg2 = m_delimiter + msg2 + m_delimiter;

		DWORD bytesWritten;
		bool bSuccess = WriteFile(
			m_writeHandle,
			msg2.data(),
			msg2.size() * sizeof(wchar_t),
			&bytesWritten,
			nullptr
		);
		if (bSuccess == false)
			throw std::runtime_error("Write operation failed.");
	}

	std::wstring Pipe::Read()
	{
		if (m_readHandle == nullptr)
			throw std::runtime_error("No active read handle.");

		std::wstring msg;
		DWORD bytesRead;
		msg.resize(m_size);
		bool bSuccess = ReadFile(
			m_readHandle,
			&msg[0],
			msg.size() * sizeof(wchar_t),
			&bytesRead,
			nullptr
		);
		if (bSuccess == false)
			throw std::runtime_error("Write operation failed");

		msg.erase(std::find(msg.begin(), msg.end(), '\0'), msg.end());

		return msg;
	}

	std::vector<std::wstring> Pipe::DelimitedRead()
	{
		std::wstring rawString = Read();
		if (m_delimiter == L"")
			return std::vector<std::wstring>{rawString};

		std::vector<std::wstring> strings = Strings::TokeniseString(rawString, m_delimiter+m_delimiter);
		if (strings.size() > 0)
		{
			strings[0] = Strings::Replace(strings[0], m_delimiter, L"");
			if (strings.size() > 1)
			{
				size_t lastIndex = strings.size() - 1;
				strings[lastIndex] = Strings::Replace(strings[lastIndex], m_delimiter, L"");
			}
		}
		return strings;
	}

	void Pipe::CloseRead()
	{
		if (m_readHandle)
		{
			CloseHandle(m_readHandle);
			m_readHandle = nullptr;
		}
	}

	void Pipe::CloseWrite()
	{
		if (m_writeHandle)
		{
			CloseHandle(m_writeHandle);
			m_writeHandle = nullptr;
		}
	}

	HANDLE Pipe::GetRead()
	{
		return m_readHandle;
	}

	HANDLE Pipe::GetWrite()
	{
		return m_writeHandle;
	}
}
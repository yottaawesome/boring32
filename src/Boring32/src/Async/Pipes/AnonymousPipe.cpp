#include "pch.hpp"
#include <stdexcept>
#include "include/Async/Pipes/AnonymousPipe.hpp"
#include "include/Strings/Strings.hpp"
#include <iostream>

namespace Boring32::Async
{
	AnonymousPipe::~AnonymousPipe()
	{
		Cleanup();
	}

	AnonymousPipe::AnonymousPipe()
	:	m_size(0),
		m_readHandle(nullptr),
		m_writeHandle(nullptr),
		m_mode(0)
	{ }

	AnonymousPipe::AnonymousPipe(const AnonymousPipe& other)
	{ 
		Copy(other);
	}

	void AnonymousPipe::operator=(const AnonymousPipe& other)
	{
		Copy(other);
	}

	void AnonymousPipe::Copy(const AnonymousPipe& other)
	{
		Cleanup();
		m_delimiter = other.m_delimiter;
		m_size = other.m_size;
		m_readHandle = other.m_readHandle;
		m_writeHandle = other.m_writeHandle;
		m_mode = other.m_mode;
	}

	AnonymousPipe::AnonymousPipe(AnonymousPipe&& other) noexcept
	{
		Move(other);
	}

	void AnonymousPipe::operator=(AnonymousPipe&& other) noexcept
	{
		Move(other);
	}

	void AnonymousPipe::Move(AnonymousPipe& other) noexcept
	{
		Cleanup();
		m_size = other.m_size;
		m_delimiter = std::move(other.m_delimiter);
		m_mode = other.m_mode;
		if (other.m_readHandle != nullptr)
			m_readHandle = std::move(other.m_readHandle);
		if (other.m_writeHandle != nullptr)
			m_writeHandle = std::move(other.m_writeHandle);
	}

	AnonymousPipe::AnonymousPipe(
		const bool inheritable, 
		const DWORD size,
		const std::wstring& delimiter
	)
	:	m_readHandle(nullptr),
		m_writeHandle(nullptr),
		m_size(size),
		m_delimiter(delimiter),
		m_mode(0)
	{
		SECURITY_ATTRIBUTES secAttrs{ 0 };
		secAttrs.nLength = sizeof(secAttrs);
		secAttrs.bInheritHandle = inheritable;
		bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, &secAttrs, size);
		if (succeeded == false)
			throw std::runtime_error("Failed to create anonymous pipe");
	}

	AnonymousPipe::AnonymousPipe(
		const DWORD size,
		const std::wstring& delimiter,
		const HANDLE readHandle,
		const HANDLE writeHandle
	)
	:	m_delimiter(delimiter),
		m_size(size),
		m_readHandle(readHandle),
		m_writeHandle(writeHandle),
		m_mode(0)
	{ }

	void AnonymousPipe::Cleanup()
	{
		m_readHandle.Close();
		m_writeHandle.Close();
	}

	void AnonymousPipe::DelimitedWrite(const std::wstring& msg)
	{
		if (m_writeHandle == nullptr)
			throw std::runtime_error("No active write handle.");

		std::wstring delimitedMsg(msg);
		if (m_delimiter != L"")
			delimitedMsg = m_delimiter + delimitedMsg + m_delimiter;

		if ((GetUsedSize() + delimitedMsg.size()) >= m_size)
			throw std::runtime_error("Pipe cannot fit message");

		DWORD bytesWritten = 0;
		bool success = WriteFile(
			m_writeHandle.GetHandle(),
			delimitedMsg.data(),
			delimitedMsg.size() * sizeof(wchar_t),
			&bytesWritten,
			nullptr
		);
		if (success == false)
			throw std::runtime_error("Write operation failed.");
	}

	void AnonymousPipe::Write(const std::wstring& msg)
	{
		if (m_writeHandle == nullptr)
			throw std::runtime_error("No active write handle.");

		if ((GetUsedSize() + msg.size()) >= m_size)
			throw std::runtime_error("Pipe cannot fit message");

		DWORD bytesWritten = 0;
		bool success = WriteFile(
			m_writeHandle.GetHandle(),
			msg.data(),
			msg.size() * sizeof(wchar_t),
			&bytesWritten,
			nullptr
		);
		if (success == false)
			throw std::runtime_error("Write operation failed.");
	}

	std::wstring AnonymousPipe::Read()
	{
		if (m_readHandle == nullptr)
			throw std::runtime_error("No active read handle.");

		std::wstring msg;
		DWORD bytesRead = 0;
		msg.resize(m_size);
		bool success = ReadFile(
			m_readHandle.GetHandle(),
			&msg[0],
			msg.size() * sizeof(wchar_t),
			&bytesRead,
			nullptr
		);
		if (success == false)
			throw std::runtime_error("Read operation failed");

		msg.erase(std::find(msg.begin(), msg.end(), '\0'), msg.end());

		return msg;
	}

	void AnonymousPipe::SetMode(const DWORD mode)
	{
		if(m_readHandle == nullptr && m_writeHandle == nullptr)
			throw std::runtime_error("Cannot set pipe mode on null pipes");

		HANDLE handleToSet = nullptr;
		if (m_readHandle != nullptr)
			handleToSet = m_readHandle.GetHandle();
		else if (m_writeHandle != nullptr)
			handleToSet = m_writeHandle.GetHandle();

		// Do not pass PIPE_READMODE_MESSAGE, as anonymous pipes are created in
		// byte mode, and cannot be changed.
		bool succeeded = SetNamedPipeHandleState(handleToSet, &m_mode, nullptr, nullptr);
		if (succeeded == false)
			throw std::runtime_error("Failed to create set pipe handle state");
	}

	std::vector<std::wstring> AnonymousPipe::DelimitedRead()
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

	void AnonymousPipe::CloseRead()
	{
		m_readHandle.Close();
	}

	void AnonymousPipe::CloseWrite()
	{
		m_writeHandle.Close();
	}

	HANDLE AnonymousPipe::GetRead()
	{
		return m_readHandle.GetHandle();
	}

	HANDLE AnonymousPipe::GetWrite()
	{
		return m_writeHandle.GetHandle();
	}

	std::wstring AnonymousPipe::GetDelimiter() const
	{
		return m_delimiter;
	}

	DWORD AnonymousPipe::GetSize() const
	{
		return m_size;
	}

	DWORD AnonymousPipe::GetUsedSize() const
	{
		DWORD charactersInPipe = 0;
		// We do this sequence of actions to determine how much space
		// is used in the passed pipe handles, if any.
		HANDLE handleToDetermineBytesAvailable = nullptr;
		if (m_readHandle != nullptr)
			handleToDetermineBytesAvailable = m_readHandle.GetHandle();
		else if (m_writeHandle != nullptr)
			handleToDetermineBytesAvailable = m_writeHandle.GetHandle();
		if (handleToDetermineBytesAvailable)
		{
			PeekNamedPipe(
				m_readHandle.GetHandle(),
				nullptr,
				0,
				nullptr,
				&charactersInPipe,
				nullptr
			);
			charactersInPipe /= sizeof(wchar_t);
		}

		return charactersInPipe;
	}

	DWORD AnonymousPipe::GetRemainingSize() const
	{
		return m_size - GetUsedSize();
	}
}
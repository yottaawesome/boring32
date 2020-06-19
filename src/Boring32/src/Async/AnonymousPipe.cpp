#include "pch.hpp"
#include <stdexcept>
#include "include/Async/AnonymousPipe.hpp"
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
		m_charactersInPipe(0)
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
		m_charactersInPipe = other.m_charactersInPipe;
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
		m_charactersInPipe = other.m_charactersInPipe;
		if (other.m_readHandle != nullptr)
			m_readHandle = std::move(other.m_readHandle);
		if (other.m_writeHandle != nullptr)
			m_writeHandle = std::move(other.m_writeHandle);
	}

	AnonymousPipe::AnonymousPipe(const bool inheritable, const DWORD size, const std::wstring& delimiter)
	:	m_readHandle(nullptr),
		m_writeHandle(nullptr),
		m_size(size),
		m_delimiter(delimiter),
		m_charactersInPipe(0)
	{
		SECURITY_ATTRIBUTES secAttrs{ 0 };
		secAttrs.nLength = sizeof(secAttrs);
		secAttrs.bInheritHandle = inheritable;
		bool succeeded = CreatePipe(&m_readHandle, &m_writeHandle, &secAttrs, size);
		//DWORD mode = PIPE_READMODE_MESSAGE;
		//SetNamedPipeHandleState(m_readHandle, &mode, nullptr, nullptr);
		//SetNamedPipeHandleState(m_writeHandle, &mode, nullptr, nullptr);
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
		m_charactersInPipe(0)
	{
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
				&m_charactersInPipe,
				nullptr
			);
			m_charactersInPipe = m_charactersInPipe / sizeof(wchar_t);
		}
	}

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

		if ((m_charactersInPipe + delimitedMsg.size()) >= m_size)
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
		m_charactersInPipe += delimitedMsg.size();
	}

	void AnonymousPipe::Write(const std::wstring& msg)
	{
		if (m_writeHandle == nullptr)
			throw std::runtime_error("No active write handle.");

		if ((m_charactersInPipe + msg.size()) >= m_size)
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
		m_charactersInPipe += bytesWritten / sizeof(wchar_t);
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

		m_charactersInPipe -= bytesRead / sizeof(wchar_t);
		msg.erase(std::find(msg.begin(), msg.end(), '\0'), msg.end());

		return msg;
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
		return m_charactersInPipe;
	}

	DWORD AnonymousPipe::GetRemainingSize() const
	{
		return m_size - m_charactersInPipe;
	}
}
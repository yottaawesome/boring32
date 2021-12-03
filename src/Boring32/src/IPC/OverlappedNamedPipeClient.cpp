#include "pch.hpp"
#include <stdexcept>
#include <format>
#include "include/Async/Pipes/OverlappedNamedPipeClient.hpp"

import boring32.error.win32error;

namespace Boring32::Async
{
	OverlappedNamedPipeClient::~OverlappedNamedPipeClient() { }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient() { }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(const std::wstring& name)
		: NamedPipeClientBase(name, FILE_FLAG_OVERLAPPED)
	{ }

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other)
		: NamedPipeClientBase(other)
	{
		Copy(other);
	}

	void OverlappedNamedPipeClient::operator=(const OverlappedNamedPipeClient& other)
	{
		Copy(other);
	}

	OverlappedNamedPipeClient::OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept
		: NamedPipeClientBase(std::move(other))
	{
		Move(other);
	}

	void OverlappedNamedPipeClient::operator=(OverlappedNamedPipeClient&& other) noexcept
	{
		Move(other);
	}

	void OverlappedNamedPipeClient::Write(std::wstring_view msg, OverlappedIo& oio)
	{
		InternalWrite(msg, oio);
	}

	bool OverlappedNamedPipeClient::Write(std::wstring_view msg, OverlappedIo& op, const std::nothrow_t&) noexcept try
	{
		InternalWrite(msg, op);
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: InternalWrite() failed: {}", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void OverlappedNamedPipeClient::InternalWrite(std::wstring_view msg, OverlappedIo& oio)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no pipe to write to");

		oio = OverlappedIo();
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
		const bool succeeded = WriteFile(
			m_handle.GetHandle(),							// pipe handle 
			&msg[0],										// message 
			static_cast<DWORD>(msg.size()*sizeof(wchar_t)),	// message length, in bytes
			nullptr,										// out bytes written 
			oio.GetOverlapped());							// overlapped 
		oio.LastError(GetLastError());
		if (succeeded == false && oio.LastError() != ERROR_IO_PENDING)
			throw Error::Win32Error(__FUNCSIG__ ": WriteFile() failed", oio.LastError());
	}

	void OverlappedNamedPipeClient::Read(const DWORD noOfCharacters, OverlappedIo& op)
	{
		return InternalRead(noOfCharacters, op);
	}

	bool OverlappedNamedPipeClient::Read(const DWORD noOfCharacters, OverlappedIo& op, const std::nothrow_t&) noexcept try
	{
		InternalRead(noOfCharacters, op);
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << std::format("{}: Read() failed: {}", __FUNCSIG__, ex.what()).c_str();
		return false;
	}

	void OverlappedNamedPipeClient::InternalRead(const DWORD noOfCharacters, OverlappedIo& oio)
	{
		if (m_handle == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": no pipe to read from");

		oio = OverlappedIo();
		oio.IoBuffer.resize(noOfCharacters);
		// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-readfile
		const bool succeeded = ReadFile(
			m_handle.GetHandle(),							// pipe handle 
			&oio.IoBuffer[0],								// buffer to receive reply 
			static_cast<DWORD>(oio.IoBuffer.size()*sizeof(wchar_t)),	// size of buffer, in bytes 
			nullptr,										// number of bytes read 
			oio.GetOverlapped());							// overlapped
		oio.LastError(GetLastError());

		if (succeeded == false)
			if (oio.LastError() != ERROR_IO_PENDING && oio.LastError() != ERROR_MORE_DATA)
				throw Error::Win32Error(__FUNCSIG__": ReadFile() failed", oio.LastError());
	}
}
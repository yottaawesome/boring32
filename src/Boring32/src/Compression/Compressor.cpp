#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Compression/Compressor.hpp"

namespace Boring32::Compression
{
	void Compressor::Close()
	{
		if (m_handle != nullptr)
		{
			CloseCompressor(m_handle);
			m_handle = nullptr;
		}
	}

	Compressor::~Compressor()
	{
		Close();
	}

	Compressor::Compressor(const CompressionType type)
	:	m_type(type),
		m_handle(nullptr)
	{
		Create();
	}

	size_t Compressor::GetCompressedSize(std::vector<std::byte>& buffer)
	{
		if (m_handle == nullptr)
			throw std::runtime_error("Compressor::GetCompressedSize(): compressor handle is null");
		if (buffer.size() == 0)
			throw std::runtime_error("Compressor::GetCompressedSize(): buffer is empty");

		size_t compressedBufferSize = 0;
		bool succeeded = Compress(
			m_handle,               //  Compressor Handle
			&buffer[0],             //  Input buffer, Uncompressed data
			buffer.size(),          //  Uncompressed data size
			nullptr,                //  Compressed Buffer
			0,                      //  Compressed Buffer size
			&compressedBufferSize);	//  Compressed Data size
		if (succeeded == false)
			throw Error::Win32Error("Compressor::GetCompressedSize(): Compress() failed", GetLastError());
		return compressedBufferSize;
	}

	void Compressor::Create()
	{
		if (m_handle == nullptr)
			throw std::runtime_error("Compressor::Create(): compressor handle is null");

		bool succeeded = CreateCompressor(
			(DWORD)m_type,
			nullptr,
			&m_handle
		);
		if (succeeded == false)
			throw Error::Win32Error("Compressor::Create(): CreateCompressor() failed", GetLastError());
	}

	void Compressor::Move(Compressor& other) noexcept
	{
		try
		{
			m_type = other.m_type;
			m_handle = other.m_handle;
			other.m_handle = nullptr;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}
	}

	void Compressor::Copy(const Compressor& other)
	{
		m_type = other.m_type;
		Create();
	}
}
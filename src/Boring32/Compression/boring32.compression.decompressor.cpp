module;

#include <vector>
#include <source_location>
#include <Windows.h>
#include <compressapi.h>

module boring32.compression.decompressor;
import boring32.error.win32error;
import boring32.error.functions;
import boring32.error.compressionerror;

namespace Boring32::Compression
{
	void Decompressor::Close()
	{
		if (m_decompressor)
		{
			CloseDecompressor(m_decompressor);
			m_decompressor = nullptr;
		}
	}

	Decompressor::~Decompressor()
	{
		Close();
	}

	Decompressor::Decompressor()
	:	m_type(CompressionType::NotSet),
		m_decompressor(nullptr)
	{ }

	Decompressor::Decompressor(const Decompressor& other)
	:	m_type(CompressionType::NotSet),
		m_decompressor(nullptr)
	{ }

	Decompressor& Decompressor::operator=(const Decompressor& other)
	{
		Copy(other);
		return *this;
	}

	void Decompressor::Copy(const Decompressor& other)
	{
		Close();
		m_type = other.m_type;
		Create();
	}

	Decompressor::Decompressor(Decompressor&& other) noexcept
	:	m_type(CompressionType::NotSet),
		m_decompressor(nullptr)
	{ }

	Decompressor& Decompressor::operator=(Decompressor&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Decompressor::Move(Decompressor& other) noexcept
	{
		Close();
		m_type = other.m_type;
		m_decompressor = other.m_decompressor;
		other.m_decompressor = nullptr;
	}

	Decompressor::Decompressor(const CompressionType type)
	:	m_type(type),
		m_decompressor(nullptr)
	{
		Create();
	}

	void Decompressor::Create()
	{
		if (m_type == CompressionType::NotSet)
			return;

		const bool succeeded = CreateDecompressor(
			static_cast<DWORD>(m_type),
			nullptr,
			&m_decompressor
		);
		if (!succeeded) Error::ThrowNested(
			Error::Win32Error(std::source_location::source_location(), "CreateDecompressor() failed", GetLastError()),
			CompressionError(std::source_location::source_location(), "An error occurred creating the decompressor")
		);
	}

	CompressionType Decompressor::GetType() const
	{
		return m_type;
	}

	size_t Decompressor::GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const
	{
		if (!m_decompressor)
			throw CompressionError(std::source_location::source_location(), "Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError(std::source_location::source_location(), "Buffer is empty");

		size_t decompressedBufferSize = 0;
		//https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-decompress
		const bool success = Decompress(
			m_decompressor,				// Decompressor handle
			&compressedBuffer[0],		// Compressed data
			compressedBuffer.size(),	// Compressed data size
			nullptr,                    // Buffer set to NULL
			0,                          // Buffer size set to 0
			&decompressedBufferSize		// Decompressed data size
		);
		if (!success) Error::ThrowNested(
			Error::Win32Error(std::source_location::source_location(), "Decompress() failed", GetLastError()),
			CompressionError(std::source_location::source_location(), "An error occurred while decompressing data")
		);
		return decompressedBufferSize;
	}

	std::vector<std::byte> Decompressor::DecompressBuffer(const std::vector<std::byte>& compressedBuffer)
	{
		if (!m_decompressor)
			throw CompressionError(std::source_location::source_location(), "Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError(std::source_location::source_location(), "Buffer is empty");

		std::vector<std::byte> returnVal(GetDecompressedSize(compressedBuffer), (std::byte)0);
		size_t decompressedBufferSize = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
		const bool succeeded = Decompress(
			m_decompressor,				//  Decompressor handle
			&compressedBuffer[0],		//  Input buffer, compressed data
			compressedBuffer.size(),	//  Compressed data size
			&returnVal[0],				//  Uncompressed buffer
			returnVal.size(),			//  Uncompressed buffer size
			&decompressedBufferSize		//  Decompressed data size
		);
		if (!succeeded) Error::ThrowNested(
			Error::Win32Error(std::source_location::source_location(), "Decompress() failed", GetLastError()),
			CompressionError(std::source_location::source_location(), "An error occurred while decompressing data")
		);

		return returnVal;
	}
}
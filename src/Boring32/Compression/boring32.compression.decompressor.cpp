module;

#include <vector>
#include <source_location>
#include <Windows.h>
#include <compressapi.h>

module boring32.compression.decompressor;
import boring32.compression.compressionerror;
import boring32.error.win32error;
import boring32.error.boring32error;
import boring32.error.functions;

namespace Boring32::Compression
{
	void Decompressor::Close()
	{
		m_decompressor.reset();
		m_type = CompressionType::NotSet;
	}

	Decompressor::~Decompressor()
	{
		Close();
	}

	Decompressor::Decompressor()
	:	m_type(CompressionType::NotSet)
	{ }

	Decompressor::Decompressor(const Decompressor& other)
	:	m_type(CompressionType::NotSet)
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
	:	m_type(CompressionType::NotSet)
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
		m_decompressor = std::move(other.m_decompressor);
	}

	Decompressor::Decompressor(const CompressionType type)
	:	m_type(type)
	{
		Create();
	}

	void Decompressor::Create()
	{
		if (m_type == CompressionType::NotSet)
			return;

		DECOMPRESSOR_HANDLE handle;
		const bool succeeded = CreateDecompressor(
			static_cast<DWORD>(m_type),
			nullptr,
			&handle
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			Error::ThrowNested(
				Error::Win32Error(
					std::source_location::current(), 
					"CreateDecompressor() failed", 
					lastError),
				CompressionError(
					std::source_location::current(), 
					"An error occurred creating the decompressor")
			);
		}
		m_decompressor = DecompressorUniquePtr(handle);
	}

	CompressionType Decompressor::GetType() const noexcept
	{
		return m_type;
	}

	size_t Decompressor::GetDecompressedSize(
		const std::vector<std::byte>& compressedBuffer
	) const
	{
		if (!m_decompressor)
			throw CompressionError(std::source_location::current(), "Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError(std::source_location::current(), "Buffer is empty");

		size_t decompressedBufferSize = 0;
		//https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-decompress
		const bool success = Decompress(
			m_decompressor.get(),		// Decompressor handle
			&compressedBuffer[0],		// Compressed data
			compressedBuffer.size(),	// Compressed data size
			nullptr,                    // Buffer set to NULL
			0,                          // Buffer size set to 0
			&decompressedBufferSize		// Decompressed data size
		);

		const auto lastError = GetLastError();
		if (lastError == ERROR_INSUFFICIENT_BUFFER)
			return decompressedBufferSize;

		Error::ThrowNested(
			Error::Win32Error(
				std::source_location::current(), 
				"Decompress() failed", 
				lastError),
			CompressionError(
				std::source_location::current(), 
				"An error occurred while decompressing data")
		);
	}

	std::vector<std::byte> Decompressor::DecompressBuffer(
		const std::vector<std::byte>& compressedBuffer
	)
	{
		if (!m_decompressor)
			throw CompressionError(std::source_location::current(), "Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError(std::source_location::current(), "Buffer is empty");

		std::vector<std::byte> returnVal(GetDecompressedSize(compressedBuffer));
		size_t decompressedBufferSize = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
		const bool succeeded = Decompress(
			m_decompressor.get(),		//  Decompressor handle
			&compressedBuffer[0],		//  Input buffer, compressed data
			compressedBuffer.size(),	//  Compressed data size
			&returnVal[0],				//  Uncompressed buffer
			returnVal.size(),			//  Uncompressed buffer size
			&decompressedBufferSize		//  Decompressed data size
		);
		if (!succeeded)
		{
			const auto lastError = GetLastError();
			Error::ThrowNested(
				Error::Win32Error(
					std::source_location::current(), 
					"CreateDecompressor() failed", 
					lastError),
				CompressionError(
					std::source_location::current(), 
					"An error occurred creating the decompressor")
			);
		}

		return returnVal;
	}
	
	DECOMPRESSOR_HANDLE Decompressor::GetHandle() const noexcept
	{
		return m_decompressor.get();
	}

	void Decompressor::Reset()
	{
		if (!m_decompressor)
			throw CompressionError(std::source_location::current(), "Decompressor handle is null");
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetdecompressor
		if (!ResetDecompressor(m_decompressor.get()))
		{
			const auto lastError = GetLastError();
			Error::ThrowNested(
				Error::Win32Error(std::source_location::current(), "ResetDecompressor() failed", lastError),
				CompressionError(std::source_location::current(), "An error occurred resetting the decompressor")
			);
		}
	}
}
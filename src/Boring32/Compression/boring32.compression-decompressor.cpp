module;

#include <source_location>

module boring32.compression:decompressor;
import :compressionerror;
import boring32.error;

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
	{ 
		Copy(other);
	}

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
	{ 
		Move(other);
	}

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
				Error::Win32Error("CreateDecompressor() failed", lastError),
				CompressionError("An error occurred creating the decompressor")
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
			throw CompressionError("Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError("Buffer is empty");

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
			Error::Win32Error( "Decompress() failed", lastError),
			CompressionError("An error occurred while decompressing data")
		);
	}

	std::vector<std::byte> Decompressor::DecompressBuffer(
		const std::vector<std::byte>& compressedBuffer
	)
	{
		if (!m_decompressor)
			throw CompressionError("Decompressor handle is null");
		if (compressedBuffer.empty())
			throw CompressionError("Buffer is empty");

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
				Error::Win32Error("CreateDecompressor() failed", lastError),
				CompressionError("An error occurred creating the decompressor"));
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
			throw CompressionError("Decompressor handle is null");
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetdecompressor
		if (!ResetDecompressor(m_decompressor.get()))
		{
			const auto lastError = GetLastError();
			Error::ThrowNested(
				Error::Win32Error("ResetDecompressor() failed", lastError),
				CompressionError("An error occurred resetting the decompressor")
			);
		}
	}
}
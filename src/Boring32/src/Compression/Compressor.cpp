module;

#include "pch.hpp"
#include "include/Error/Win32Error.hpp"

module boring32.compression.compressor;

// For reference see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api-in-block-mode
namespace Boring32::Compression
{
	void Compressor::Close()
	{
		if (m_compressor != nullptr)
		{
			CloseCompressor(m_compressor);
			m_type = CompressionType::NotSet;
			m_compressor = nullptr;
		}
	}

	Compressor::Compressor()
	:	m_type(CompressionType::NotSet),
		m_compressor(nullptr)
	{ }

	Compressor::~Compressor()
	{
		Close();
	}

	Compressor::Compressor(const CompressionType type)
	:	m_type(type),
		m_compressor(nullptr)
	{
		Create();
	}

	Compressor::Compressor(const Compressor& other)
	:	m_type(CompressionType::NotSet),
		m_compressor(nullptr)
	{
		Copy(other);
	}

	Compressor& Compressor::operator=(const Compressor other)
	{
		Copy(other);
		return *this;
	}

	void Compressor::Copy(const Compressor& other)
	{
		Close();
		m_type = other.m_type;
		Create();
	}

	Compressor::Compressor(Compressor&& other) noexcept
	:	m_type(CompressionType::NotSet),
		m_compressor(nullptr)
	{
		Move(other);
	}

	Compressor& Compressor::operator=(Compressor&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Compressor::Move(Compressor& other) noexcept
	{
		try
		{
			Close();
			m_type = other.m_type;
			m_compressor = other.m_compressor;
			other.m_compressor = nullptr;
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}
	}

	size_t Compressor::GetCompressedSize(const std::vector<std::byte>& buffer) const
	{
		if (m_compressor == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": compressor handle is null");
		if (buffer.size() == 0)
			throw std::runtime_error(__FUNCSIG__  ": buffer is empty");

		size_t compressedBufferSize = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
		const bool succeeded = Compress(
			m_compressor,           //  Compressor Handle
			&buffer[0],             //  Input buffer, Uncompressed data
			buffer.size(),          //  Uncompressed data size
			nullptr,                //  Compressed Buffer
			0,                      //  Compressed Buffer size
			&compressedBufferSize	//  Compressed Data size
		);	
		const DWORD lastError = GetLastError();
		if (succeeded == false && lastError != ERROR_INSUFFICIENT_BUFFER)
			throw Error::Win32Error(__FUNCSIG__ ": Compress() failed", GetLastError());

		return compressedBufferSize;
	}

	CompressionType Compressor::GetType() const
	{
		return m_type;
	}

	std::vector<std::byte> Compressor::CompressBuffer(const std::vector<std::byte>& buffer)
	{
		if (m_compressor == nullptr)
			throw std::runtime_error("Compressor::CompressBuffer(): compressor handle is null");
		if (buffer.size() == 0)
			throw std::runtime_error("Compressor::CompressBuffer(): buffer is empty");

		std::vector<std::byte> returnVal(GetCompressedSize(buffer), (std::byte)0);
		size_t compressedBufferSize = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
		bool succeeded = Compress(
			m_compressor,           //  Compressor Handle
			&buffer[0],             //  Input buffer, Uncompressed data
			buffer.size(),          //  Uncompressed data size
			&returnVal[0],          //  Compressed Buffer
			returnVal.size(),       //  Compressed Buffer size
			&compressedBufferSize);	//  Compressed Data size
		if (succeeded == false)
			throw Error::Win32Error("Compressor::CompressBuffer(): Compress() failed", GetLastError());

		return returnVal;
	}

	void Compressor::Create()
	{
		if (m_type != CompressionType::NotSet)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-createcompressor
			bool succeeded = CreateCompressor(
				(DWORD)m_type,	// Algorithm
				nullptr,		// AllocationRoutines
				&m_compressor	// CompressorHandle
			);
			if (succeeded == false)
				throw Error::Win32Error("Compressor::Create(): CreateCompressor() failed", GetLastError());
		}
	}

	COMPRESSOR_HANDLE Compressor::GetHandle() const noexcept
	{
		return m_compressor;
	}
}
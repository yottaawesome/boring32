module;

#include "pch.hpp"

module boring32.compression.decompressor;
import boring32.error.win32error;

namespace Boring32::Compression
{
	void Decompressor::Close()
	{
		if (m_decompressor != nullptr)
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
		if (m_type != CompressionType::NotSet)
		{
			bool succeeded = CreateDecompressor(
				(DWORD)m_type,
				nullptr,
				&m_decompressor
			);
			if (succeeded == false)
				throw Error::Win32Error(std::source_location::source_location(), "Failed to create decompressor", GetLastError());
		}
	}

	CompressionType Decompressor::GetType() const
	{
		return m_type;
	}

	size_t Decompressor::GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const
	{
		if (m_decompressor == nullptr)
			throw std::runtime_error("Decompressor::DecompressBuffer(): decompressor handle is null");
		if (compressedBuffer.size() == 0)
			throw std::runtime_error("Decompressor::DecompressBuffer(): buffer is empty");

		size_t decompressedBufferSize = 0;
		//https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-decompress
		bool success = Decompress(
			m_decompressor,				// Decompressor handle
			&compressedBuffer[0],		// Compressed data
			compressedBuffer.size(),	// Compressed data size
			nullptr,                    // Buffer set to NULL
			0,                          // Buffer size set to 0
			&decompressedBufferSize);	// Decompressed data size
		if (success == false)
			throw Error::Win32Error(std::source_location::source_location(), "Decompressor::GetDecompressedSize(): Decompress() failed", GetLastError());
		return decompressedBufferSize;
	}

	std::vector<std::byte> Decompressor::DecompressBuffer(const std::vector<std::byte>& compressedBuffer)
	{
		if (m_decompressor == nullptr)
			throw std::runtime_error("Decompressor::DecompressBuffer(): decompressor handle is null");
		if (compressedBuffer.size() == 0)
			throw std::runtime_error("Decompressor::DecompressBuffer(): buffer is empty");

		std::vector<std::byte> returnVal(GetDecompressedSize(compressedBuffer), (std::byte)0);
		size_t decompressedBufferSize = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
		bool succeeded = Decompress(
			m_decompressor,				//  Decompressor handle
			&compressedBuffer[0],		//  Input buffer, compressed data
			compressedBuffer.size(),	//  Compressed data size
			&returnVal[0],				//  Uncompressed buffer
			returnVal.size(),			//  Uncompressed buffer size
			&decompressedBufferSize);	//  Decompressed data size
		if (succeeded == false)
			throw Error::Win32Error(std::source_location::source_location(), "Decompressor::DecompressBuffer(): Decompressor() failed", GetLastError());

		return returnVal;
	}
}
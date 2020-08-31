#include "pch.hpp"
#include "include/Error/Win32Error.hpp"
#include "include/Compression/Decompressor.hpp"

namespace Boring32::Compression
{
	void Decompressor::Close()
	{
		if (m_handle != nullptr)
		{
			CloseDecompressor(m_handle);
			m_handle = nullptr;
		}
	}

	Decompressor::~Decompressor()
	{
		Close();
	}

	Decompressor::Decompressor(const CompressionType type)
	:	m_type(type),
		m_handle(nullptr)
	{
		Create();
	}

	void Decompressor::Create()
	{
		bool succeeded = CreateCompressor(
			(DWORD)m_type,
			nullptr,
			&m_handle
		);
		if (succeeded == false)
			throw Error::Win32Error("Failed to create compressor", GetLastError());
	}
}
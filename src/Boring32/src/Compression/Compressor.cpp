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

	void Compressor::Create()
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
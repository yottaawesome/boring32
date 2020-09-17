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

	Decompressor::Decompressor()
	:	m_type(CompressionType::NotSet),
		m_handle(nullptr)
	{ }

	Decompressor::Decompressor(const Decompressor& other)
	:	m_type(CompressionType::NotSet),
		m_handle(nullptr)
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
		m_handle(nullptr)
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
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	Decompressor::Decompressor(const CompressionType type)
	:	m_type(type),
		m_handle(nullptr)
	{
		Create();
	}

	void Decompressor::Create()
	{
		if (m_type != CompressionType::NotSet)
		{
			bool succeeded = CreateCompressor(
				(DWORD)m_type,
				nullptr,
				&m_handle
			);
			if (succeeded == false)
				throw Error::Win32Error("Failed to create decompressor", GetLastError());
		}
	}

	CompressionType Decompressor::GetType() const
	{
		return m_type;
	}
}
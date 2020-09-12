#pragma once
#include <vector>
#include <compressapi.h>
#include "CompressionType.hpp"

namespace Boring32::Compression
{
	class Compressor
	{
		public:
			virtual ~Compressor();
			Compressor(const CompressionType type);

		public:
			virtual void Close();
			virtual size_t GetCompressedSize(std::vector<std::byte>& buffer);

		protected:
			virtual void Create();
			virtual void Move(Compressor& other) noexcept;
			virtual void Copy(const Compressor& other);

		protected:
			CompressionType m_type;
			COMPRESSOR_HANDLE m_handle;
	};
}
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

			Compressor(const Compressor& other);
			virtual Compressor& operator=(const Compressor other);

			Compressor(Compressor&& other) noexcept;
			virtual Compressor& operator=(Compressor&& other) noexcept;

		public:
			virtual void Close();
			virtual size_t GetCompressedSize(std::vector<std::byte>& buffer);
			virtual CompressionType GetType();

		protected:
			virtual void Create();
			virtual void Move(Compressor& other) noexcept;
			virtual void Copy(const Compressor& other);

		protected:
			CompressionType m_type;
			COMPRESSOR_HANDLE m_compressor;
	};
}
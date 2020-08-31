#pragma once
#include <compressapi.h>
#include "CompressionType.hpp"

namespace Boring32::Compression
{
	class Decompressor
	{
		public:
			virtual ~Decompressor();
			Decompressor(const CompressionType type);

		public:
			virtual void Close();

		protected:
			virtual void Create();

		protected:
			CompressionType m_type;
			DECOMPRESSOR_HANDLE m_handle;
	};
}
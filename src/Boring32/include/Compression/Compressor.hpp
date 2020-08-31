#pragma once
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

		protected:
			virtual void Create();

		protected:
			CompressionType m_type;
			COMPRESSOR_HANDLE m_handle;
	};
}
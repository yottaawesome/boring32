#pragma once
#include <vector>
#include <compressapi.h>
#include "CompressionType.hpp"

namespace Boring32::Compression
{
	class Decompressor
	{
		public:
			virtual ~Decompressor();
			Decompressor();

			Decompressor(const Decompressor& other);
			virtual Decompressor& operator=(const Decompressor& other);

			Decompressor(Decompressor&& other) noexcept;
			virtual Decompressor& operator=(Decompressor&& other) noexcept;

			Decompressor(const CompressionType type);

		public:
			virtual void Close();

			/// <summary>
			///		Returns the type of algorithm of this decompressor.
			/// </summary>
			/// <returns>The algorithm used by this decompressor.</returns>
			[[nodiscard]] virtual CompressionType GetType() const;

			[[nodiscard]] virtual size_t GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const;

			[[nodiscard]] virtual std::vector<std::byte> DecompressBuffer(const std::vector<std::byte>& compressedBuffer);

		protected:
			virtual void Create();
			virtual void Copy(const Decompressor& other);
			virtual void Move(Decompressor& other) noexcept;

		protected:
			CompressionType m_type;
			DECOMPRESSOR_HANDLE m_decompressor;
	};
}
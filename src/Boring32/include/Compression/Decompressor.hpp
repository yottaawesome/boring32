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

			/// <summary>
			///		Returns the uncompressed size, in bytes, of the compressed buffer specified in the parameter.
			/// </summary>
			/// <param name="buffer">The buffer to determine the uncompressed size of.</param>
			/// <returns>The uncompressed size, in bytes, of the compressed buffer specified in the parameter</returns>
			[[nodiscard]] virtual size_t GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const;

			/// <summary>
			///		Returns a buffer that is the decompressed data of the input argument, buffer.
			/// </summary>
			/// <param name="buffer">The buffer to decompress.</param>
			/// <returns>The decompressed buffer.</returns>
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
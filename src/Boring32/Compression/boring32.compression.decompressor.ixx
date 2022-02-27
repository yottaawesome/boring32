module;

#include <vector>
#include <Windows.h>
#include <compressapi.h>

export module boring32.compression.decompressor;
import boring32.compression.compressiontype;
import boring32.compression.deleters;

export namespace Boring32::Compression
{
	class Decompressor
	{
		public:
			virtual ~Decompressor();
			Decompressor();
			Decompressor(const Decompressor& other);
			Decompressor(Decompressor&& other) noexcept;
			Decompressor(const CompressionType type);

		public:
			virtual Decompressor& operator=(const Decompressor& other);
			virtual Decompressor& operator=(Decompressor&& other) noexcept;

		public:
			virtual void Close();

			/// <summary>
			///		Returns the type of algorithm of this decompressor.
			/// </summary>
			/// <returns>The algorithm used by this decompressor.</returns>
			[[nodiscard]] virtual CompressionType GetType() const noexcept;

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

			[[nodiscard]] virtual DECOMPRESSOR_HANDLE GetHandle() const noexcept;

			virtual void Reset();

		protected:
			virtual void Create();
			virtual void Copy(const Decompressor& other);
			virtual void Move(Decompressor& other) noexcept;

		protected:
			CompressionType m_type;
			DecompressorUniquePtr m_decompressor;
	};
}
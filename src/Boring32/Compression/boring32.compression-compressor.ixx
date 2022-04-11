module;

#include <vector>
#include <Windows.h>
#include <compressapi.h>

export module boring32.compression:compressor;
import :compressiontype;
import :deleters;

/// <summary>
/// For reference of the Compression API, see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api
/// </summary>
export namespace Boring32::Compression
{
	class Compressor
	{
		public:
			virtual ~Compressor();
			Compressor();
			Compressor(const Compressor& other);
			Compressor(Compressor&& other) noexcept;
			Compressor(const CompressionType type);

		public:
			virtual Compressor& operator=(const Compressor other);
			virtual Compressor& operator=(Compressor&& other) noexcept;

		public:
			/// <summary>
			///		Returns the size, in bytes, of the compressed buffer specified in the parameter.
			/// </summary>
			/// <param name="buffer">The buffer to determine the compressed size of.</param>
			/// <returns>The size, in bytes, of the compressed buffer specified in the parameter</returns>
			/// <exception cref="CompressionError">Thrown if an empty buffer is passed, or no compressor is set, or compression fails.</exception>
			[[nodiscard]] virtual size_t GetCompressedSize(
				const std::vector<std::byte>& buffer
			) const;

			/// <summary>
			///		Returns the type of algorithm of this compressor.
			/// </summary>
			/// <returns>The algorithm used by this compressor.</returns>
			[[nodiscard]] virtual CompressionType GetType() const noexcept;

			/// <summary>
			///		Returns a buffer that is the compressed data of the input argument, buffer.
			/// </summary>
			/// <param name="buffer">The buffer to compress.</param>
			/// <returns>The compressed buffer.</returns>
			[[nodiscard]] virtual std::vector<std::byte> CompressBuffer(
				const std::vector<std::byte>& buffer
			);

			/// <summary>
			///		Releases all resources associated with this object.
			/// </summary>
			virtual void Close();

			[[nodiscard]] virtual COMPRESSOR_HANDLE GetHandle() const noexcept;
			
			virtual void Reset();

		protected:
			virtual void Create();
			virtual void Move(Compressor& other) noexcept;
			virtual void Copy(const Compressor& other);

		protected:
			CompressionType m_type;
			CompressorUniquePtr m_compressor;
	};
}
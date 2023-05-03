export module boring32.compression:compressor;
import <iostream>;
import <vector>;
import <win32.hpp>;
import :compressiontype;
import :deleters;
import :compressionerror;
import boring32.error;

/// <summary>
/// For reference of the Compression API, see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api
// For reference see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api-in-block-mode
/// </summary>
export namespace Boring32::Compression
{
	class Compressor final
	{
		// The Six
		public:
			~Compressor()
			{
				Close();
			}

			Compressor() = default;

			Compressor(const Compressor& other)
			{
				Copy(other);
			}
			Compressor& operator=(const Compressor& other)
			{
				Copy(other);
				return *this;
			}

			Compressor(Compressor&& other) noexcept
				: m_type(CompressionType::NotSet)
			{
				Move(other);
			}
			Compressor& operator=(Compressor&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			Compressor(const CompressionType type)
				: m_type(type)
			{
				Create();
			}	

		public:
			/// <summary>
			///		Returns the size, in bytes, of the compressed buffer specified in the parameter.
			/// </summary>
			/// <param name="buffer">The buffer to determine the compressed size of.</param>
			/// <returns>The size, in bytes, of the compressed buffer specified in the parameter</returns>
			/// <exception cref="CompressionError">Thrown if an empty buffer is passed, or no compressor is set, or compression fails.</exception>
			[[nodiscard]] size_t GetCompressedSize(
				const std::vector<std::byte>& buffer
			) const
			{
				if (!m_compressor)
					throw CompressionError("Compressor handle is null");
				if (buffer.empty())
					throw CompressionError("Buffer is empty");

				size_t compressedBufferSize = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
				const bool succeeded = Compress(
					m_compressor.get(),     //  Compressor Handle
					&buffer[0],             //  Input buffer, Uncompressed data
					buffer.size(),          //  Uncompressed data size
					nullptr,                //  Compressed Buffer
					0,                      //  Compressed Buffer size
					&compressedBufferSize	//  Compressed Data size
				);
				const DWORD lastError = GetLastError();
				if (!succeeded && lastError != ERROR_INSUFFICIENT_BUFFER)
				{
					Error::ThrowNested(
						Error::Win32Error("Compress() failed", lastError),
						CompressionError("An error occurred calculating the compressed size"));
				}

				return compressedBufferSize;
			}

			/// <summary>
			///		Returns the type of algorithm of this compressor.
			/// </summary>
			/// <returns>The algorithm used by this compressor.</returns>
			[[nodiscard]] CompressionType GetType() const noexcept
			{
				return m_type;
			}

			/// <summary>
			///		Returns a buffer that is the compressed data of the input argument, buffer.
			/// </summary>
			/// <param name="buffer">The buffer to compress.</param>
			/// <returns>The compressed buffer.</returns>
			[[nodiscard]] std::vector<std::byte> CompressBuffer(
				const std::vector<std::byte>& buffer
			)
			{
				if (!m_compressor)
					throw CompressionError("Compressor handle is null");
				if (buffer.empty())
					throw CompressionError("Buffer is empty");

				std::vector<std::byte> returnVal(GetCompressedSize(buffer));
				size_t compressedBufferSize = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
				const bool succeeded = Compress(
					m_compressor.get(),     //  Compressor Handle
					&buffer[0],             //  Input buffer, Uncompressed data
					buffer.size(),          //  Uncompressed data size
					&returnVal[0],          //  Compressed Buffer
					returnVal.size(),       //  Compressed Buffer size
					&compressedBufferSize	//  Compressed Data size
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("Compress() failed", lastError),
						CompressionError("An error occurred compressing")
					);
				}

				return returnVal;
			}

			/// <summary>
			///		Releases all resources associated with this object.
			/// </summary>
			void Close()
			{
				m_compressor.reset();
				m_type = CompressionType::NotSet;
			}

			[[nodiscard]] COMPRESSOR_HANDLE GetHandle() const noexcept
			{
				return m_compressor.get();
			}
			
			void Reset()
			{
				if (!m_compressor)
					throw CompressionError("Compressor handle is null");
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetcompressor
				if (!ResetCompressor(m_compressor.get()))
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("ResetCompressor() failed", lastError),
						CompressionError("An error occurred resetting the compressor")
					);
				}
			}

		private:
			void Create()
			{
				if (m_type == CompressionType::NotSet)
					return;
				COMPRESSOR_HANDLE handle;
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-createcompressor
				const bool succeeded = CreateCompressor(
					static_cast<DWORD>(m_type),	// Algorithm
					nullptr,		// AllocationRoutines
					&handle			// CompressorHandle
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("CreateCompressor() failed", lastError),
						CompressionError("An error occurred creating the compressor")
					);
				}
				m_compressor = CompressorUniquePtr(handle);
			}

			void Move(Compressor& other) noexcept try
			{
				Close();
				m_type = other.m_type;
				m_compressor = std::move(other.m_compressor);
			}
			catch (const std::exception& ex)
			{
				std::wcerr << ex.what() << std::endl;
			}

			void Copy(const Compressor& other)
			{
				Close();
				m_type = other.m_type;
				Create();
			}

		private:
			CompressionType m_type = CompressionType::NotSet;
			CompressorUniquePtr m_compressor;
	};
}
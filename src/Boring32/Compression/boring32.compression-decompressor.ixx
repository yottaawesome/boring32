export module boring32.compression:decompressor;
import <vector>;
import <win32.hpp>;
import boring32.error;
import :compressiontype;
import :deleters;
import :compressionerror;

export namespace Boring32::Compression
{
	class Decompressor final
	{
		// The Six
		public:
			~Decompressor()
			{
				Close();
			}

			Decompressor() = default;

			Decompressor(const Decompressor& other)
			{
				Copy(other);
			}
			Decompressor& operator=(const Decompressor& other)
			{
				Copy(other);
				return *this;
			}

			Decompressor(Decompressor&& other) noexcept
			{
				Move(other);
			}
			Decompressor& operator=(Decompressor&& other) noexcept
			{
				Move(other);
				return *this;
			}
			
		public:
			Decompressor(const CompressionType type)
				: m_type(type)
			{
				Create();
			}

		public:
			void Close()
			{
				m_decompressor.reset();
				m_type = CompressionType::NotSet;
			}

			/// <summary>
			///		Returns the type of algorithm of this decompressor.
			/// </summary>
			/// <returns>The algorithm used by this decompressor.</returns>
			[[nodiscard]] CompressionType GetType() const noexcept
			{
				return m_type;
			}

			/// <summary>
			///		Returns the uncompressed size, in bytes, of the compressed buffer specified in the parameter.
			/// </summary>
			/// <param name="buffer">The buffer to determine the uncompressed size of.</param>
			/// <returns>The uncompressed size, in bytes, of the compressed buffer specified in the parameter</returns>
			[[nodiscard]] size_t GetDecompressedSize(
				const std::vector<std::byte>& compressedBuffer
			) const
			{
				if (!m_decompressor)
					throw CompressionError("Decompressor handle is null");
				if (compressedBuffer.empty())
					throw CompressionError("Buffer is empty");

				size_t decompressedBufferSize = 0;
				//https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-decompress
				const bool success = Decompress(
					m_decompressor.get(),		// Decompressor handle
					&compressedBuffer[0],		// Compressed data
					compressedBuffer.size(),	// Compressed data size
					nullptr,                    // Buffer set to NULL
					0,                          // Buffer size set to 0
					&decompressedBufferSize		// Decompressed data size
				);

				const auto lastError = GetLastError();
				if (lastError == ERROR_INSUFFICIENT_BUFFER)
					return decompressedBufferSize;

				Error::ThrowNested(
					Error::Win32Error("Decompress() failed", lastError),
					CompressionError("An error occurred while decompressing data")
				);
			}

			/// <summary>
			///		Returns a buffer that is the decompressed data of the input argument, buffer.
			/// </summary>
			/// <param name="buffer">The buffer to decompress.</param>
			/// <returns>The decompressed buffer.</returns>
			[[nodiscard]] std::vector<std::byte> DecompressBuffer(
				const std::vector<std::byte>& compressedBuffer
			)
			{
				if (!m_decompressor)
					throw CompressionError("Decompressor handle is null");
				if (compressedBuffer.empty())
					throw CompressionError("Buffer is empty");

				std::vector<std::byte> returnVal(GetDecompressedSize(compressedBuffer));
				size_t decompressedBufferSize = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
				const bool succeeded = Decompress(
					m_decompressor.get(),		//  Decompressor handle
					&compressedBuffer[0],		//  Input buffer, compressed data
					compressedBuffer.size(),	//  Compressed data size
					&returnVal[0],				//  Uncompressed buffer
					returnVal.size(),			//  Uncompressed buffer size
					&decompressedBufferSize		//  Decompressed data size
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("CreateDecompressor() failed", lastError),
						CompressionError("An error occurred creating the decompressor"));
				}

				return returnVal;
			}

			[[nodiscard]] DECOMPRESSOR_HANDLE GetHandle() const noexcept
			{
				return m_decompressor.get();
			}

			void Reset()
			{
				if (!m_decompressor)
					throw CompressionError("Decompressor handle is null");
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetdecompressor
				if (!ResetDecompressor(m_decompressor.get()))
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("ResetDecompressor() failed", lastError),
						CompressionError("An error occurred resetting the decompressor")
					);
				}
			}

		private:
			void Create()
			{
				if (m_type == CompressionType::NotSet)
					return;

				DECOMPRESSOR_HANDLE handle;
				const bool succeeded = CreateDecompressor(
					static_cast<DWORD>(m_type),
					nullptr,
					&handle
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					Error::ThrowNested(
						Error::Win32Error("CreateDecompressor() failed", lastError),
						CompressionError("An error occurred creating the decompressor")
					);
				}
				m_decompressor = DecompressorUniquePtr(handle);
			}

			void Copy(const Decompressor& other)
			{
				Close();
				m_type = other.m_type;
				Create();
			}

			void Move(Decompressor& other) noexcept
			{
				Close();
				m_type = other.m_type;
				m_decompressor = std::move(other.m_decompressor);
			}

		private:
			CompressionType m_type = CompressionType::NotSet;
			DecompressorUniquePtr m_decompressor;
	};
}
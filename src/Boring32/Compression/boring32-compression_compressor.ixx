export module boring32:compression_compressor;
import boring32.shared;
import boring32.error;
import :compression_deleters;
import :compression_compressionerror;

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
				: m_type(Win32::CompressionType::NotSet)
			{
				Move(other);
			}
			Compressor& operator=(Compressor&& other) noexcept
			{
				Move(other);
				return *this;
			}

		public:
			Compressor(const Win32::CompressionType type)
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
				const bool succeeded = Win32::Compress(
					m_compressor.get(),     //  Compressor Handle
					&buffer[0],             //  Input buffer, Uncompressed data
					buffer.size(),          //  Uncompressed data size
					nullptr,                //  Compressed Buffer
					0,                      //  Compressed Buffer size
					&compressedBufferSize	//  Compressed Data size
				);
				const Win32::DWORD lastError = Win32::GetLastError();
				if (!succeeded && lastError != Win32::ErrorCodes::InsufficientBuffer)
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
			[[nodiscard]] Win32::CompressionType GetType() const noexcept
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
				const bool succeeded = Win32::Compress(
					m_compressor.get(),     //  Compressor Handle
					&buffer[0],             //  Input buffer, Uncompressed data
					buffer.size(),          //  Uncompressed data size
					&returnVal[0],          //  Compressed Buffer
					returnVal.size(),       //  Compressed Buffer size
					&compressedBufferSize	//  Compressed Data size
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
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
				m_type = Win32::CompressionType::NotSet;
			}

			[[nodiscard]] Win32::COMPRESSOR_HANDLE GetHandle() const noexcept
			{
				return m_compressor.get();
			}
			
			void Reset()
			{
				if (!m_compressor)
					throw CompressionError("Compressor handle is null");
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetcompressor
				if (!Win32::ResetCompressor(m_compressor.get()))
				{
					const auto lastError = Win32::GetLastError();
					Error::ThrowNested(
						Error::Win32Error("ResetCompressor() failed", lastError),
						CompressionError("An error occurred resetting the compressor")
					);
				}
			}

		private:
			void Create()
			{
				if (m_type == Win32::CompressionType::NotSet)
					return;
				Win32::COMPRESSOR_HANDLE handle;
				// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-createcompressor
				const bool succeeded = Win32::CreateCompressor(
					static_cast<DWORD>(m_type),	// Algorithm
					nullptr,		// AllocationRoutines
					&handle			// CompressorHandle
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
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
				// ICE...
				std::wcerr << ex.what() << std::endl;
			}

			void Copy(const Compressor& other)
			{
				Close();
				m_type = other.m_type;
				Create();
			}

		private:
			Win32::CompressionType m_type = Win32::CompressionType::NotSet;
			CompressorUniquePtr m_compressor;
	};
}
export module boring32:compression.compressor;
import std;
import :win32;
import :error;
import :compression.deleters;
import :compression.compressionerror;

// For reference of the Compression API, see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api
// For reference see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api-in-block-mode
export namespace Boring32::Compression
{
	struct Compressor final
	{
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

		Compressor(const Win32::CompressionType type)
			: m_type(type)
		{
			Create();
		}	

		///	Returns the size, in bytes, of the compressed buffer specified in the parameter.
		[[nodiscard]] size_t GetCompressedSize(const std::vector<std::byte>& buffer) const
		{
			if (not m_compressor)
				throw CompressionError("Compressor handle is null");
			if (buffer.empty())
				throw CompressionError("Buffer is empty");

			size_t compressedBufferSize = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
			bool succeeded = Win32::Compress(
				m_compressor.get(),     //  Compressor Handle
				&buffer[0],             //  Input buffer, Uncompressed data
				buffer.size(),          //  Uncompressed data size
				nullptr,                //  Compressed Buffer
				0,                      //  Compressed Buffer size
				&compressedBufferSize	//  Compressed Data size
			);
			Win32::DWORD lastError = Win32::GetLastError();
			if (not succeeded && lastError != Win32::ErrorCodes::InsufficientBuffer)
			{
				Error::ThrowNested(
					Error::Win32Error(lastError, "Compress() failed"),
					CompressionError("An error occurred calculating the compressed size"));
			}

			return compressedBufferSize;
		}

		///	Returns the type of algorithm of this compressor.
		[[nodiscard]] Win32::CompressionType GetType() const noexcept
		{
			return m_type;
		}

		///	Returns a buffer that is the compressed data of the input argument, buffer.
		[[nodiscard]] std::vector<std::byte> CompressBuffer(const std::vector<std::byte>& buffer)
		{
			if (not m_compressor)
				throw CompressionError("Compressor handle is null");
			if (buffer.empty())
				throw CompressionError("Buffer is empty");

			std::vector<std::byte> returnVal(GetCompressedSize(buffer));
			size_t compressedBufferSize = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
			bool succeeded = Win32::Compress(
				m_compressor.get(),     //  Compressor Handle
				&buffer[0],             //  Input buffer, Uncompressed data
				buffer.size(),          //  Uncompressed data size
				&returnVal[0],          //  Compressed Buffer
				returnVal.size(),       //  Compressed Buffer size
				&compressedBufferSize	//  Compressed Data size
			);
			if (not succeeded)
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "Compress() failed"),
					CompressionError("An error occurred compressing")
				);
			}

			return returnVal;
		}

		///	Releases all resources associated with this object.
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
			if (not m_compressor)
				throw CompressionError("Compressor handle is null");
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetcompressor
			if (not Win32::ResetCompressor(m_compressor.get()))
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "ResetCompressor() failed"),
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
			bool succeeded = Win32::CreateCompressor(
				static_cast<DWORD>(m_type),	// Algorithm
				nullptr,		// AllocationRoutines
				&handle			// CompressorHandle
			);
			if (not succeeded)
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "CreateCompressor() failed"),
					CompressionError("An error occurred creating the compressor")
				);
			}
			m_compressor = CompressorUniquePtr(handle);
		}

		void Move(Compressor& other) noexcept 
		try
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

		Win32::CompressionType m_type = Win32::CompressionType::NotSet;
		CompressorUniquePtr m_compressor;
	};
}
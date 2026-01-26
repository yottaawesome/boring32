export module boring32:compression.decompressor;
import std;
import boring32.win32;
import :error;
import :compression.deleters;
import :compression.compressionerror;

export namespace Boring32::Compression
{
	struct Decompressor final
	{
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
			
		Decompressor(const Win32::CompressionType type)
			: m_type(type)
		{
			Create();
		}

		void Close()
		{
			m_decompressor.reset();
			m_type = Win32::CompressionType::NotSet;
		}

		///	Returns the type of algorithm of this decompressor.
		[[nodiscard]] Win32::CompressionType GetType() const noexcept
		{
			return m_type;
		}

		///	Returns the uncompressed size, in bytes, of the compressed buffer specified in the parameter.
		[[nodiscard]] size_t GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const
		{
			if (not m_decompressor)
				throw CompressionError("Decompressor handle is null");
			if (compressedBuffer.empty())
				throw CompressionError("Buffer is empty");

			size_t decompressedBufferSize = 0;
			//https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-decompress
			bool success = Win32::Decompress(
				m_decompressor.get(),		// Decompressor handle
				&compressedBuffer[0],		// Compressed data
				compressedBuffer.size(),	// Compressed data size
				nullptr,                    // Buffer set to NULL
				0,                          // Buffer size set to 0
				&decompressedBufferSize		// Decompressed data size
			);
			auto lastError = Win32::GetLastError();
			if (lastError == Win32::ErrorCodes::InsufficientBuffer)
				return decompressedBufferSize;

			Error::ThrowNested(
				Error::Win32Error(lastError, "Decompress() failed"),
				CompressionError("An error occurred while decompressing data")
			);
		}

		///	Returns a buffer that is the decompressed data of the input argument, buffer.
		[[nodiscard]] std::vector<std::byte> DecompressBuffer(const std::vector<std::byte>& compressedBuffer)
		{
			if (not m_decompressor)
				throw CompressionError("Decompressor handle is null");
			if (compressedBuffer.empty())
				throw CompressionError("Buffer is empty");

			std::vector<std::byte> returnVal(GetDecompressedSize(compressedBuffer));
			size_t decompressedBufferSize = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
			bool succeeded = Win32::Decompress(
				m_decompressor.get(),		//  Decompressor handle
				&compressedBuffer[0],		//  Input buffer, compressed data
				compressedBuffer.size(),	//  Compressed data size
				&returnVal[0],				//  Uncompressed buffer
				returnVal.size(),			//  Uncompressed buffer size
				&decompressedBufferSize		//  Decompressed data size
			);
			if (not succeeded)
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "CreateDecompressor() failed"),
					CompressionError("An error occurred creating the decompressor"));
			}

			return returnVal;
		}

		[[nodiscard]] Win32::DECOMPRESSOR_HANDLE GetHandle() const noexcept
		{
			return m_decompressor.get();
		}

		void Reset()
		{
			if (not m_decompressor)
				throw CompressionError("Decompressor handle is null");
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetdecompressor
			if (not Win32::ResetDecompressor(m_decompressor.get()))
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "ResetDecompressor() failed"),
					CompressionError("An error occurred resetting the decompressor")
				);
			}
		}

		private:
		void Create()
		{
			if (m_type == Win32::CompressionType::NotSet)
				return;

			Win32::DECOMPRESSOR_HANDLE handle;
			const bool succeeded = Win32::CreateDecompressor(
				static_cast<Win32::DWORD>(m_type),
				nullptr,
				&handle
			);
			if (not succeeded)
			{
				Error::ThrowNested(
					Error::Win32Error(Win32::GetLastError(), "CreateDecompressor() failed"),
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

		Win32::CompressionType m_type = Win32::CompressionType::NotSet;
		DecompressorUniquePtr m_decompressor;
	};
}
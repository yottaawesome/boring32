export module boring32:compression.decompressor;
import std;
import :win32;
import :error;
import :compression.deleters;

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

		auto operator=(const Decompressor& other) -> Decompressor&
		{
			Copy(other);
			return *this;
		}

		Decompressor(Decompressor&& other) noexcept
		{
			Move(other);
		}
		auto operator=(Decompressor&& other) noexcept -> Decompressor&
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
		[[nodiscard]] 
		auto GetType() const noexcept -> Win32::CompressionType
		{
			return m_type;
		}

		///	Returns the uncompressed size, in bytes, of the compressed buffer specified in the parameter.
		[[nodiscard]] 
		auto GetDecompressedSize(const std::vector<std::byte>& compressedBuffer) const -> size_t
		{
			if (not m_decompressor)
				throw Error::Boring32Error("Decompressor handle is null");
			if (compressedBuffer.empty())
				throw Error::Boring32Error("Buffer is empty");

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
			throw Error::Win32Error(lastError, "Decompress() failed");
		}

		///	Returns a buffer that is the decompressed data of the input argument, buffer.
		[[nodiscard]] 
		auto DecompressBuffer(const std::vector<std::byte>& compressedBuffer) -> std::vector<std::byte>
		{
			if (not m_decompressor)
				throw Error::Boring32Error("Decompressor handle is null");
			if (compressedBuffer.empty())
				throw Error::Boring32Error("Buffer is empty");

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
				auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "CreateDecompressor() failed");
			}

			return returnVal;
		}

		[[nodiscard]] 
		auto GetHandle() const noexcept -> Win32::DECOMPRESSOR_HANDLE
		{
			return m_decompressor.get();
		}

		void Reset()
		{
			if (not m_decompressor)
				throw Error::Boring32Error("Decompressor handle is null");
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetdecompressor
			if (not Win32::ResetDecompressor(m_decompressor.get()))
			{
				auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "ResetDecompressor() failed");
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
				auto lastError = Win32::GetLastError();
				throw Error::Win32Error(lastError, "CreateDecompressor() failed");
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
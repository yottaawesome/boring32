export module boring32:compression.compressor;
import std;
import :win32;
import :error;
import :compression.deleters;

// For reference of the Compression API, see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api
// For reference see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api-in-block-mode
export namespace Boring32::Compression
{
	class Compressor final
	{
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

		auto operator=(const Compressor& other) -> Compressor&
		{
			Copy(other);
			return *this;
		}

		Compressor(Compressor&& other) noexcept
			: m_type(Win32::CompressionType::NotSet)
		{
			Move(other);
		}
		auto operator=(Compressor&& other) noexcept -> Compressor&
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
		[[nodiscard]] 
		auto GetCompressedSize(const std::vector<std::byte>& buffer) const -> size_t
		{
			if (not m_compressor)
				throw Error::Boring32Error("Compressor handle is null");
			if (buffer.empty())
				throw Error::Boring32Error("Buffer is empty");

			auto compressedBufferSize = size_t{};
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-compress
			bool succeeded = Win32::Compress(
				m_compressor.get(),     //  Compressor Handle
				&buffer[0],             //  Input buffer, Uncompressed data
				buffer.size(),          //  Uncompressed data size
				nullptr,                //  Compressed Buffer
				0,                      //  Compressed Buffer size
				&compressedBufferSize	//  Compressed Data size
			);
			auto lastError = Win32::GetLastError();
			if (not succeeded and lastError != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error{ lastError, "Compress() failed" };

			return compressedBufferSize;
		}

		///	Returns the type of algorithm of this compressor.
		[[nodiscard]] 
		auto GetType() const noexcept -> Win32::CompressionType
		{
			return m_type;
		}

		///	Returns a buffer that is the compressed data of the input argument, buffer.
		[[nodiscard]] 
		auto CompressBuffer(const std::vector<std::byte>& buffer) -> std::vector<std::byte>
		{
			if (not m_compressor)
				throw Error::Boring32Error{ "Compressor handle is null" };
			if (buffer.empty())
				throw Error::Boring32Error{ "Buffer is empty" };

			auto returnVal = std::vector<std::byte>(GetCompressedSize(buffer));
			auto compressedBufferSize = size_t{};
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
				throw Error::Win32Error{ Win32::GetLastError(), "Compress() failed" };

			return returnVal;
		}

		///	Releases all resources associated with this object.
		void Close()
		{
			m_compressor.reset();
			m_type = Win32::CompressionType::NotSet;
		}

		[[nodiscard]] 
		auto GetHandle() const noexcept -> Win32::COMPRESSOR_HANDLE
		{
			return m_compressor.get();
		}
			
		void Reset()
		{
			if (not m_compressor)
				throw Error::Boring32Error("Compressor handle is null");
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-resetcompressor
			if (not Win32::ResetCompressor(m_compressor.get()))
				throw Error::Win32Error{ Win32::GetLastError(), "ResetCompressor() failed" };
		}

	private:
		void Create()
		{
			if (m_type == Win32::CompressionType::NotSet)
				return;
			auto handle = Win32::COMPRESSOR_HANDLE{};
			// https://docs.microsoft.com/en-us/windows/win32/api/compressapi/nf-compressapi-createcompressor
			bool succeeded = Win32::CreateCompressor(
				static_cast<DWORD>(m_type),	// Algorithm
				nullptr,		// AllocationRoutines
				&handle			// CompressorHandle
			);
			if (not succeeded)
				throw Error::Win32Error{ Win32::GetLastError(), "CreateCompressor() failed" };
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
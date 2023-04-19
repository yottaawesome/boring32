module boring32.compression:compressor;
import :compressionerror;
import boring32.error;
import <iostream>;

// For reference see: https://docs.microsoft.com/en-us/windows/win32/cmpapi/using-the-compression-api-in-block-mode
namespace Boring32::Compression
{
	void Compressor::Close()
	{
		m_compressor.reset();
		m_type = CompressionType::NotSet;
	}

	Compressor::~Compressor()
	{
		Close();
	}

	Compressor::Compressor(const CompressionType type)
	:	m_type(type)
	{
		Create();
	}

	Compressor::Compressor(const Compressor& other)
	{
		Copy(other);
	}

	Compressor& Compressor::operator=(const Compressor other)
	{
		Copy(other);
		return *this;
	}

	void Compressor::Copy(const Compressor& other)
	{
		Close();
		m_type = other.m_type;
		Create();
	}

	Compressor::Compressor(Compressor&& other) noexcept
	:	m_type(CompressionType::NotSet)
	{
		Move(other);
	}

	Compressor& Compressor::operator=(Compressor&& other) noexcept
	{
		Move(other);
		return *this;
	}

	void Compressor::Move(Compressor& other) noexcept try
	{
		Close();
		m_type = other.m_type;
		m_compressor = std::move(other.m_compressor);
	}
	catch (const std::exception& ex)
	{
		std::wcerr << ex.what() << std::endl;
	}

	size_t Compressor::GetCompressedSize(const std::vector<std::byte>& buffer) const
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

	CompressionType Compressor::GetType() const noexcept
	{
		return m_type;
	}

	std::vector<std::byte> Compressor::CompressBuffer(const std::vector<std::byte>& buffer)
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

	void Compressor::Create()
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

	COMPRESSOR_HANDLE Compressor::GetHandle() const noexcept
	{
		return m_compressor.get();
	}

	void Compressor::Reset()
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
}
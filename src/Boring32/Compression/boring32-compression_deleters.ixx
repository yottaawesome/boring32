export module boring32:compression_deleters;
import boring32.shared;

export namespace Boring32::Compression 
{
	struct CompressorDeleter final
	{
		void operator()(Win32::COMPRESSOR_HANDLE handle) const noexcept
		{
			Win32::CloseCompressor(handle);
		}
	};
	using CompressorUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::COMPRESSOR_HANDLE>, CompressorDeleter>;
	
	struct DecompressorDeleter final
	{
		void operator()(Win32::DECOMPRESSOR_HANDLE handle) const noexcept
		{
			Win32::CloseDecompressor(handle);
		}
	};
	using DecompressorUniquePtr = std::unique_ptr<std::remove_pointer_t<Win32::DECOMPRESSOR_HANDLE>, DecompressorDeleter>;
}

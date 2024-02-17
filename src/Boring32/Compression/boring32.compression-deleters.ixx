export module boring32.compression:deleters;
import boring32.shared;

export namespace Boring32::Compression 
{
	struct CompressorDeleter final
	{
		void operator()(Win32::COMPRESSOR_HANDLE handle)
		{
			Win32::CloseCompressor(handle);
		}
	};
	using CompressorUniquePtr = std::unique_ptr<
		std::remove_pointer<Win32::COMPRESSOR_HANDLE>::type,
		CompressorDeleter>;
	
	struct DecompressorDeleter final
	{
		void operator()(Win32::DECOMPRESSOR_HANDLE handle)
		{
			Win32::CloseDecompressor(handle);
		}
	};
	using DecompressorUniquePtr = std::unique_ptr<
		std::remove_pointer<Win32::DECOMPRESSOR_HANDLE>::type,
		DecompressorDeleter>;
}
export module boring32.compression:deleters;
import std;

import <win32.hpp>;

export namespace Boring32::Compression 
{
	struct CompressorDeleter final
	{
		void operator()(COMPRESSOR_HANDLE handle)
		{
			CloseCompressor(handle);
		}
	};
	using CompressorUniquePtr = std::unique_ptr<
		std::remove_pointer<COMPRESSOR_HANDLE>::type, 
		CompressorDeleter>;
	
	struct DecompressorDeleter final
	{
		void operator()(DECOMPRESSOR_HANDLE handle)
		{
			CloseDecompressor(handle);
		}
	};
	using DecompressorUniquePtr = std::unique_ptr<
		std::remove_pointer<DECOMPRESSOR_HANDLE>::type, 
		DecompressorDeleter>;
}
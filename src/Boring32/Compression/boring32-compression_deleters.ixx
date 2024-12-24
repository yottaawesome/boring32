export module boring32:compression_deleters;
import boring32.shared;
import :raii;

export namespace Boring32::Compression 
{
	using CompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseCompressor>;
	using DecompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseDecompressor>;
}

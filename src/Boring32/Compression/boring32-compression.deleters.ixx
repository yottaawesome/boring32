export module boring32:compression.deleters;
import std;
import boring32.win32;
import :raii;

export namespace Boring32::Compression 
{
	using CompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseCompressor>;
	using DecompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseDecompressor>;
}

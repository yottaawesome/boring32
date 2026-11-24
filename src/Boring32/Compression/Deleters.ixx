export module Boring32:Compression.Deleters;
import std;
import :Win32;
import :Raii;

export namespace Boring32::Compression 
{
	using CompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseCompressor>;
	using DecompressorUniquePtr = RAII::IndirectUniquePtr<Win32::COMPRESSOR_HANDLE, Win32::CloseDecompressor>;
}

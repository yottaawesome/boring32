export module boring32.compression:compressiontype;
import <Windows.h>;

export namespace Boring32::Compression
{
	enum class CompressionType : DWORD
	{
		NotSet = 0,
		MSZIP = COMPRESSION_FORMAT_LZNT1,
		XPRESS = COMPRESSION_FORMAT_XPRESS,
		XPRESSHuffman = COMPRESSION_FORMAT_XPRESS_HUFF,
		LZMS = COMPRESSION_FORMAT_XP10
	};
}
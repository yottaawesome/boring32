#pragma once

namespace Boring32::Compression
{
	enum class CompressionType
	{
		MSZIP = 2,
		XPRESS = 3,
		XPRESSHuffman = 4,
		LZMS = 5
	};
}
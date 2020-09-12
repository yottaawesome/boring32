#pragma once

namespace Boring32::Compression
{
	enum class CompressionType
	{
		NotSet = 0,
		MSZIP = 2,
		XPRESS = 3,
		XPRESSHuffman = 4,
		LZMS = 5
	};
}
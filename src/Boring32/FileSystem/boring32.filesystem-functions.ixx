module;

#include <string>

export module boring32.filesystem:functions;

export namespace Boring32::FileSystem
{
	// Requires linking with Version.lib
	std::wstring GetFileVersion(const std::wstring& filePath);
}
module;

#include <string>

export module boring32.filesystem:functions;

export namespace Boring32::FileSystem
{
	// Requires linking with Version.lib
	std::wstring GetFileVersion(const std::wstring& filePath);
	void CreateFileDirectory(const std::wstring& path);
	void CreateFileDirectory(const std::wstring& path, const std::wstring& dacl);
}
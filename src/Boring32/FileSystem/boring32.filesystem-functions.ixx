export module boring32.filesystem:functions;
import <string>;

export namespace Boring32::FileSystem
{
	// Requires linking with Version.lib
	std::wstring GetFileVersion(const std::wstring& filePath);
	// Requires the parent directories to exist
	void CreateFileDirectory(const std::wstring& path);
	void CreateFileDirectory(const std::wstring& path, const std::wstring& dacl);
	void MoveNamedFile(
		const std::wstring& oldFile, 
		const std::wstring& newFile,
		unsigned flags = 0
	);
}
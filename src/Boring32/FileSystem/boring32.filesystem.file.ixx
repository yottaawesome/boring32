module;

#include <string>

export module boring32.filesystem.file;
import boring32.raii.win32handle;

export namespace Boring32::FileSystem
{
	class File
	{
		public:
			virtual ~File();
			File();
			File(std::wstring file);

		public:
			virtual void Close();

		protected:
			virtual void InternalOpen();

		protected:
			std::wstring m_fileName;
			Raii::Win32Handle m_fileHandle;
	};
}
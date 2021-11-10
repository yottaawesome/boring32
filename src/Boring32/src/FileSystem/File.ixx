module;

#include <string>
#include "include/Raii/Win32Handle.hpp"

export module boring32.filesystem.file;

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
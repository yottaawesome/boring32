module;

#include <string>
#include "include/Raii/Win32Handle.hpp"

export module boring32.async.filemapping;

export namespace Boring32::Async
{
	class FileMapping
	{
		public:
			virtual ~FileMapping();
			FileMapping(const bool isInheritable, const size_t maxSizeLow);
			FileMapping(
				const bool isInheritable, 
				const std::wstring name, 
				const size_t maxSize
			);
			FileMapping(
				const bool isInheritable, 
				const std::wstring name, 
				const size_t maxSize,
				const DWORD desiredAccess
			);

		public:
			virtual void Close();
			virtual const std::wstring GetName() const noexcept final;
			virtual HANDLE GetNativeHandle() const noexcept final;
			virtual const Raii::Win32Handle GetHandle() const noexcept final;
			virtual size_t GetFileSize() const final;
			
		protected:
			virtual void CreateOrOpen(
				const bool create,
				const DWORD desiredAccess,
				const bool isInheritable
			);

		protected:
			Raii::Win32Handle m_mapFile;
			size_t m_maxSize;
			std::wstring m_name;
	};
}

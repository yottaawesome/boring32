#pragma once
#include <Windows.h>
#include <string>

namespace Win32Utils::Process
{
	class DynamicLinkLibrary
	{
		public:
			DynamicLinkLibrary(const std::wstring& path);
			virtual ~DynamicLinkLibrary();
			virtual const std::wstring& GetPath();
			virtual const HMODULE GetHandle();
			virtual void* Resolve(const std::wstring& path);

		protected:
			const std::wstring path;
			HMODULE libraryHandle;
	};

	class MemoryMappedFile
	{
		public:
			MemoryMappedFile(
				const std::wstring& name,
				const UINT maxSize,
				const bool createNewMutex,
				const bool inheritable
			);
			virtual ~MemoryMappedFile();
			virtual void* GetViewPointer();
			virtual bool Initialised();

		public:
			MemoryMappedFile(const MemoryMappedFile& other) = delete;
			virtual void operator=(const MemoryMappedFile& other) = delete;

		protected:
			virtual void Cleanup();

		protected:
			bool m_initialised;
			bool m_createFile;
			bool m_inheritable;
			const std::wstring m_mmfName;
			const UINT m_maxSize;
			HANDLE m_MapFile;
			void* m_View;
	};

	class Mutex
	{
		public:
			Mutex(const std::wstring_view name, const bool create, const bool acquire, const bool inheritable);
			virtual ~Mutex();
			virtual void Lock();
			virtual void Unlock();
	
		public:
			Mutex(const Mutex& other) = delete;
			virtual void operator=(const Mutex& other) = delete;

		protected:
			const std::wstring m_name;
			const bool m_inheritable;
			const bool m_created;
			bool m_locked;
			HANDLE m_mutex;
	};
}
#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	/// <summary>
	///		Represents a <a href="https://docs.microsoft.com/en-us/dotnet/standard/io/memory-mapped-files">Win32 memory-mapped file</a>,
	///		which allows processes to share memory. This is a copyable
	///		and movable object.
	/// </summary>
	class MemoryMappedFile
	{
		// Constructors and destructor
		public:
			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object.
			/// </summary>
			virtual ~MemoryMappedFile();

			/// <summary>
			///		Default constructor. Does not initialise the MemoryMappedFile.
			/// </summary>
			MemoryMappedFile();

			/// <summary>
			///		Creates a new memory mapped file.
			/// </summary>
			/// <param name="name">
			///		The name of the memory mapped file to create or open.
			/// </param>
			/// <param name="maxSize">
			///		The maximum size of the memory mapped file.
			/// </param>
			/// <param name="inheritable">
			///		Whether the acquired handle can be inherited by child processes.
			/// </param>
			MemoryMappedFile(
				std::wstring name,
				const UINT maxSize,
				const bool inheritable
			);

			/// <summary>
			///		Opens a memory mapped file.
			/// </summary>
			/// <param name="name">
			///		The name of the memory mapped file to create or open.
			/// </param>
			/// <param name="maxSize">
			///		The maximum size of the memory mapped file.
			/// </param>
			/// <param name="inheritable">
			///		Whether the acquired handle can be inherited by child processes.
			/// </param>
			/// <param name="desiredAccess">
			///		The desired access to open the file. See: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
			/// </param>
			MemoryMappedFile(
				std::wstring name,
				const UINT maxSize,
				const bool inheritable,
				const DWORD desiredAccess
			);

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			MemoryMappedFile(const MemoryMappedFile& other);

			/// <summary>
			///		Unmaps the view of the MemoryMappedFile and releases
			///		the handle owned by this object before duplicating
			///		from the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			virtual MemoryMappedFile& operator=(const MemoryMappedFile& other);

			/// <summary>
			///		Move constructor.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to move.</param>
			MemoryMappedFile(MemoryMappedFile&& other) noexcept;

			/// <summary>
			///		Move assignment.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to move.</param>
			virtual MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept;

		// API
		public:
			/// <summary>
			///		Gets the view pointer of the memory mapped file.
			/// </summary>
			/// <returns>The view object.</returns>
			virtual void* GetViewPointer();

			/// <summary>
			///		Get the name of this MemoryMappedFile.
			/// </summary>
			/// <returns>The name of this MemoryMappedFile.</returns>
			virtual const std::wstring& GetName() const;

			/// <summary>
			///		Get whether this MemoryMappedFile can be inherited
			///		by child processes.
			/// </summary>
			/// <returns></returns>
			virtual bool IsInheritable() const;

		protected:
			/// <summary>
			///		Unlocks the mutex if it is currently owned, releases the 
			///		mutex handle, and clears the underlying pointer.
			/// </summary>
			virtual void Close();

			/// <summary>
			///		Moves the MemoryMappedFile on the RHS into this object.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to move.</param>
			virtual void Move(MemoryMappedFile& other) noexcept;

			/// <summary>
			///		Duplicates the specified MemoryMappedFile.
			/// </summary>
			/// <param name="other">The MemoryMappedFile to duplicate.</param>
			virtual void Copy(const MemoryMappedFile& other);

		protected:
			std::wstring m_name;
			UINT m_maxSize;
			Raii::Win32Handle m_mapFile;
			void* m_view;
	};
}
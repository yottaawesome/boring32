export module boring32.async:filemapping;
import boring32.raii;
import <string>;
import <win32.hpp>;

export namespace Boring32::Async
{
	enum class FileMapAccess : unsigned long
	{
		All = FILE_MAP_ALL_ACCESS,
		Execute = FILE_MAP_EXECUTE,
		Read = FILE_MAP_READ,
		Write = FILE_MAP_WRITE
	};

	class FileMapping
	{
		public:
			virtual ~FileMapping();
			FileMapping() = default;
			FileMapping(
				const bool isInheritable, 
				const size_t maxSize, 
				const DWORD pageProtection
			);

			/// <summary>
			/// Create a new page-backed file mapping.
			/// </summary>
			/// <param name="isInheritable"></param>
			/// <param name="name"></param>
			/// <param name="maxSize"></param>
			/// <param name="pageProtection"></param>
			FileMapping(
				const bool isInheritable, 
				const std::wstring name, 
				const size_t maxSize, 
				const DWORD pageProtection
			);

			/// <summary>
			/// Open an existing named page file with specific access.
			/// </summary>
			/// <param name="name"></param>
			/// <param name="desiredAccess"></param>
			/// <param name="isInheritable"></param>
			/// <param name="maxSize"></param>
			FileMapping(
				const std::wstring name,
				const FileMapAccess desiredAccess,
				const bool isInheritable,
				const size_t maxSize
			);
			FileMapping(const FileMapping& other);
			FileMapping(FileMapping&& other) noexcept;

		public:
			virtual FileMapping& operator=(const FileMapping& other);
			virtual FileMapping& operator=(FileMapping&& other) noexcept;

		public:
			virtual void Close();
			virtual const std::wstring GetName() const noexcept final;
			virtual HANDLE GetNativeHandle() const noexcept final;
			virtual const RAII::Win32Handle GetHandle() const noexcept final;
			virtual size_t GetFileSize() const final;
			
		protected:
			virtual void CreateOrOpen(
				const bool create,
				const FileMapAccess desiredAccess,
				const bool isInheritable
			);
			virtual FileMapping& Copy(const FileMapping& other);
			virtual FileMapping& Move(FileMapping& other) noexcept;

		protected:
			RAII::Win32Handle m_fileMapping;
			size_t m_maxSize = 0;
			std::wstring m_name;
			DWORD m_pageProtection = 0;
	};
}

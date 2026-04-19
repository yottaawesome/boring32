export module boring32:async.memorymappedview;
import std;
import :win32;
import :error;
import :async.filemapping;

export namespace Boring32::Async
{
	/// RAII wrapper for a mapped view of a file mapping object.
	/// Move-only: each instance owns a unique address range.
	class MemoryMappedView final
	{
	public:
		~MemoryMappedView()
		{
			Close(std::nothrow);
		}

		MemoryMappedView() = default;

		MemoryMappedView(
			const FileMapping& mapping,
			Win32::FileMapAccess access,
			Win32::DWORD offsetHigh = 0,
			Win32::DWORD offsetLow = 0,
			size_t bytesToMap = 0
		)
		{
			m_view = Win32::MapViewOfFile(
				mapping.GetNativeHandle(),
				static_cast<Win32::DWORD>(access),
				offsetHigh,
				offsetLow,
				bytesToMap
			);
			if (not m_view)
				throw Error::Win32Error{Win32::GetLastError(), "MapViewOfFile() failed"};
		}

		MemoryMappedView(const MemoryMappedView&) = delete;
		auto operator=(const MemoryMappedView&) -> MemoryMappedView& = delete;

		MemoryMappedView(MemoryMappedView&& other) noexcept
			: m_view(other.m_view)
		{
			other.m_view = nullptr;
		}

		auto operator=(MemoryMappedView&& other) noexcept -> MemoryMappedView&
		{
			if (this != &other)
			{
				Close(std::nothrow);
				m_view = other.m_view;
				other.m_view = nullptr;
			}
			return *this;
		}

		auto GetPointer() const noexcept -> void*
		{
			return m_view;
		}

		auto Close() -> void
		{
			if (m_view and not Win32::UnmapViewOfFile(m_view))
				throw Error::Win32Error{Win32::GetLastError(), "UnmapViewOfFile() failed"};
			m_view = nullptr;
		}

		auto Close(const std::nothrow_t&) noexcept -> void
		try
		{
			Close();
		}
		catch (const std::exception& ex)
		{
			std::wcerr << std::format("MemoryMappedView::Close() failed: {}\n", ex.what()).c_str();
		}

	private:
		void* m_view = nullptr;
	};

	/// Typed convenience wrapper over FileMapping + MemoryMappedView.
	template<typename T>
	class TypedMemoryMappedView final
	{
	public:
		TypedMemoryMappedView(
			const FileMapping& mapping,
			Win32::FileMapAccess access,
			bool placementNew = true
		) : m_view(mapping, access)
		{
			if (placementNew)
				new (m_view.GetPointer()) T();
		}

		auto GetView() noexcept -> T*
		{
			return static_cast<T*>(m_view.GetPointer());
		}

		auto GetView() const noexcept -> const T*
		{
			return static_cast<const T*>(m_view.GetPointer());
		}

	private:
		MemoryMappedView m_view;
	};
}
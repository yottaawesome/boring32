export module boring32:async_memorymappedview;
import boring32.shared;
import :async_memorymappedfile;

export namespace Boring32::Async
{
	template<typename T>
	struct MemoryMappedView final
	{
		MemoryMappedView(const std::wstring& name, const bool create, const bool inheritable)
		: m_mappedMemory(name, sizeof(T), create, inheritable)
		{
			m_view = (T*) m_mappedMemory.GetViewPointer();
			if (create)
				m_view = new (m_view) T();
		}

		T* GetView()
		{
			return m_view;
		}

		private:
		MemoryMappedFile m_mappedMemory;
		T* m_view = nullptr;
	};
}
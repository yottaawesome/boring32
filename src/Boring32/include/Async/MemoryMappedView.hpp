#pragma once
#include <string>
#include "MemoryMappedFile.hpp"

namespace Boring32::Async
{
	template<typename T>
	class MemoryMappedView
	{
		public:
			MemoryMappedView(const std::wstring& name, const bool create, const bool inheritable)
			:	m_mappedMemory(
					name,
					sizeof(T),
					create,
					inheritable
				),
				m_view(nullptr)
			{
				m_view = (T*) m_mappedMemory.GetViewPointer();
				if (create)
					m_view = new (m_view) T();
			}

			T* GetView()
			{
				return m_view;
			}

		protected:
			MemoryMappedFile m_mappedMemory;
			T* m_view;
	};
}
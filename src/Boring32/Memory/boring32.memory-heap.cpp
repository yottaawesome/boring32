module boring32.memory:heap;
import boring32.error;

namespace Boring32::Memory
{
	Heap::~Heap() 
	{ 
		Destroy(); 
	}
	
	Heap::Heap(HANDLE heap)
	{
		if (!heap)
			throw Error::Boring32Error("Must pass in a valid heap pointer");
		m_heap = heap;
	}

	void Heap::Destroy()
	{
		// Don't destroy the process' default heap, 
		// only special-purpose heaps
		if (m_heap != GetProcessHeap())
			HeapDestroy(m_heap);
	}

	size_t Heap::Compact()
	{
		const size_t size = HeapCompact(
			m_heap,
			0
		);
		if (size == 0)
		{
			const auto lastError = GetLastError();
			if (lastError == NO_ERROR)
				return 0;
			throw Error::Win32Error("HeapCompact() failed", lastError);
		}
		return size;
	}

	bool Heap::Validate(
		void* const ptr,
		const bool synchronised
	) noexcept
	{
		return HeapValidate(
			m_heap,
			synchronised ? 0 : 0x01, //HEAP_NO_SERIALISE,
			ptr
		);
	}
}
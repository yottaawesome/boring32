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
		HANDLE defaultHeap = GetProcessHeap();
		if (m_heap != defaultHeap)
			HeapDestroy(m_heap);
	}
}
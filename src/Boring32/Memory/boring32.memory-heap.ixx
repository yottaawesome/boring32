export module boring32.memory:heap;
import boring32.error;
import <win32.hpp>;

export namespace Boring32::Memory
{
	class Heap final
	{
		// The six
		public:
			~Heap()
			{
				Destroy();
			}

			Heap() = delete;
			
			Heap(const Heap&) = delete;
			Heap& operator=(const Heap&) = delete;

			Heap(Heap&& other) noexcept
			{
				Move(other);
			}
			Heap& operator=(Heap&& other) noexcept
			{
				return Move(other);
			}

		public:
			Heap(HANDLE heap)
			{
				if (!heap)
					throw Error::Boring32Error("Must pass in a valid heap pointer");
				m_heap = heap;
			}

		public:
			void Destroy()
			{
				// Don't destroy the process' default heap, 
				// only special-purpose heaps
				if (m_heap && m_heap != GetProcessHeap())
				{
					HeapDestroy(m_heap);
					m_heap = nullptr;
				}
			}

			size_t Compact()
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

			bool Validate(
				void* const ptr = nullptr, 
				const bool synchronised = true
			) noexcept
			{
				return HeapValidate(
					m_heap,
					synchronised ? 0 : 0x01, //HEAP_NO_SERIALISE,
					ptr
				);
			}

			void Lock()
			{
				const bool success = HeapLock(
					m_heap
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapLock() failed", lastError);
				}
			}
			
			void Unlock()
			{
				const bool success = HeapUnlock(
					m_heap
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapUnlock() failed", lastError);
				}
			}

		private:
			Heap& Move(Heap& other)
			{
				Destroy();
				m_heap = other.m_heap;
				other.m_heap = nullptr;
				return *this;
			}

		private:
			HANDLE m_heap = nullptr;
	};
}
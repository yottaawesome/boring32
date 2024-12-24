export module boring32:memory_heap;
import boring32.shared;
import :error;

export namespace Boring32::Memory
{
	using HeapCreateOptions = Win32::HeapCreateOptions;
	using HeapAllocOptions = Win32::HeapAllocOptions;

	struct Heap final
	{
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

		Heap(Win32::HANDLE heap)
		{
			if (not heap)
				throw Error::Boring32Error("Must pass in a valid heap pointer");
			m_heap = heap;
		}

		Heap(Win32::DWORD options, Win32::DWORD initialSize, Win32::DWORD maxSize = 0)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapcreate
			m_heap = Win32::HeapCreate(options, initialSize, maxSize);
			if (auto lastError = Win32::GetLastError(); not m_heap)
				throw Error::Win32Error("HeapCreate() failed", lastError);
		}

		void Destroy()
		{
			// Don't destroy the process' default heap, 
			// only special-purpose heaps
			if (not m_heap or m_heap == Win32::GetProcessHeap())
				return;
			Win32::HeapDestroy(m_heap);
			m_heap = nullptr;
		}

		size_t Compact()
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapcompact
			size_t size = Win32::HeapCompact(m_heap, 0);
			if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::NoError)
				throw Error::Win32Error("HeapCompact() failed", lastError);
			return size;
		}

		bool Validate(void* const ptr = nullptr, bool synchronised = true) noexcept
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapvalidate
			return Win32::HeapValidate(
				m_heap,
				synchronised ? 0 : 1, //HEAP_NO_SERIALISE,
				ptr
			);
		}

		void Lock()
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heaplock
			if (auto lastError = Win32::GetLastError(); not Win32::HeapLock(m_heap))
				throw Error::Win32Error("HeapLock() failed", lastError);
		}
			
		void Unlock()
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapunlock
			if (auto lastError = Win32::GetLastError(); not Win32::HeapUnlock(m_heap))
				throw Error::Win32Error("HeapUnlock() failed", lastError);
		}

		[[nodiscard]] void* New(size_t bytes, Win32::DWORD options = 0)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
			void* allocation = Win32::HeapAlloc(m_heap, options, bytes);
			if (auto lastError = Win32::GetLastError(); not allocation)
				throw Error::Win32Error("HeapAlloc() failed", lastError);
			return allocation;
		}

		template<typename T, typename...TArgs>
		[[nodiscard]] T* New(Win32::DWORD options = 0, TArgs&&... args)
		{
			void* ptr = New(sizeof(T), options);
			return new(ptr) T(std::forward<TArgs>(args)...);
		}

		template<typename T>
		struct Deleter
		{
			Deleter(Heap& h) : m_h(h) {}
			void operator()(T* t)
			{
				m_h.Delete((void*)t);
			}

			private:
			Heap& m_h;
		};

		template<typename T, typename...Args>
		[[nodiscard]] std::unique_ptr<T, Deleter<T>> NewPtr(const Args&... args)
		{
			void* ptr = New(sizeof(T), 0);
			return std::unique_ptr<T, Deleter<T>> (new(ptr) T(args...), Deleter<T>(*this));
		}

		void Delete(void* ptr, Win32::DWORD options = 0)
		{
			// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree
			if (auto lastError = Win32::GetLastError(); ptr and not Win32::HeapFree(m_heap, options, ptr))
				throw Error::Win32Error("HeapFree() failed", lastError);
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-getprocessheap
		const static inline Win32::HANDLE ProcessHeap = Win32::GetProcessHeap();

		private:
		Heap& Move(Heap& other)
		{
			Destroy();
			m_heap = other.m_heap;
			other.m_heap = nullptr;
			return *this;
		}
		Win32::HANDLE m_heap = nullptr;
	};
}

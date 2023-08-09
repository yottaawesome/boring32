export module boring32.memory:heap;
import std;

import <win32.hpp>;
import boring32.error;

export namespace Boring32::Memory
{
	enum class HeapCreateOptions : DWORD
	{
		EnableExecute = HEAP_CREATE_ENABLE_EXECUTE,
		GenerateExceptions = HEAP_GENERATE_EXCEPTIONS,
		NoSerialise = HEAP_NO_SERIALIZE
	};

	enum class HeapAllocOptions : DWORD
	{
		None = 0,
		ZeroMemory = HEAP_ZERO_MEMORY,
		GenerateExceptions = HEAP_GENERATE_EXCEPTIONS,
		NoSerialise = HEAP_NO_SERIALIZE
	};

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

			Heap(const DWORD options, const DWORD initialSize, const DWORD maxSize = 0)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapcreate
				m_heap = HeapCreate(
					options,
					initialSize,
					maxSize
				);
				if (!m_heap)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapCreate() failed", lastError);
				}
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
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapcompact
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
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapvalidate
				return HeapValidate(
					m_heap,
					synchronised ? 0 : 0x01, //HEAP_NO_SERIALISE,
					ptr
				);
			}

			void Lock()
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heaplock
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
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapunlock
				const bool success = HeapUnlock(
					m_heap
				);
				if (!success)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapUnlock() failed", lastError);
				}
			}

			[[nodiscard]] void* New(const size_t bytes, const DWORD options = 0)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc
				void* allocation = HeapAlloc(m_heap, options, bytes);
				if (!allocation)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapAlloc() failed", lastError);
				}
				return allocation;
			}

			template<typename T>
			[[nodiscard]] T* New(const DWORD options = 0)
			{
				void* ptr = New(sizeof(T), options);
				return new(ptr) T();
			}

			template<typename T, typename...Args>
			[[nodiscard]] T* New(const DWORD options = 0, Args&... args)
			{
				void* ptr = New(sizeof(T), options);
				return new(ptr) T(args...);
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
				return std::unique_ptr<T, Deleter<T>> (
					new(ptr) T(args...), 
					Deleter<T>(*this)
				);
			}

			void Delete(void* ptr, const DWORD options = 0)
			{
				// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree
				if (ptr && !HeapFree(m_heap, options, ptr))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("HeapFree() failed", lastError);
				}
			}

			const static HANDLE ProcessHeap;

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

	// https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-getprocessheap
	const HANDLE Heap::ProcessHeap = GetProcessHeap();
}
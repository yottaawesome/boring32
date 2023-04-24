export module boring32.memory:heap;
import <win32.hpp>;

export namespace Boring32::Memory
{
	class Heap final
	{
		public:
			~Heap();
			Heap(HANDLE heap);

		public:
			void Destroy();
			size_t Compact();
			bool Validate(
				void* const ptr = nullptr, 
				const bool synchronised = true
			) noexcept;
			void Lock();
			void Unlock();

		private:
			HANDLE m_heap = nullptr;
	};
}
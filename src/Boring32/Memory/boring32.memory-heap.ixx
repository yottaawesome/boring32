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

		private:
			HANDLE m_heap = nullptr;
	};
}
export module boring32:datastructures.singlylinkedlist;
import <malloc.h>;
import std;
import :win32;
import :error;

export namespace Boring32::DataStructures
{
	template<typename T>
	struct ListElement 
	{
		Win32::SLIST_ENTRY EntryInfo;
		std::shared_ptr<T> Item;
	};

	/// See https://docs.microsoft.com/en-us/windows/win32/sync/interlocked-singly-linked-lists
	/// and https://docs.microsoft.com/en-us/windows/win32/sync/using-singly-linked-lists
	template<typename T>
	class SinglyLinkedList final
	{
	public:
		~SinglyLinkedList()
		{
			Close();
		}

		SinglyLinkedList()
		{
			m_listHeader = reinterpret_cast<Win32::PSLIST_HEADER>(
				_aligned_malloc(
					sizeof(Win32::SLIST_HEADER),
					Win32::MemoryAllocationAlignment
				)
			);
			if (not m_listHeader)
				throw Error::Boring32Error{ "_aligned_malloc() failed" };
			Win32::InitializeSListHead(m_listHeader);
		}

		SinglyLinkedList(const SinglyLinkedList& other) = delete;

		SinglyLinkedList(SinglyLinkedList&& other) noexcept
			: SinglyLinkedList()
		{
			Move(other);
		}

		SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;
		auto operator=(SinglyLinkedList&& other) noexcept -> SinglyLinkedList&
		{
			return Move(other);
		}

		void Close()
		{
			if (m_listHeader)
			{
				EmptyList();
				_aligned_free(m_listHeader);
				m_listHeader = nullptr;
			}
		}

		using ListElementPtr = ListElement<T>*;

		template<typename...Args>
		void Add(Args&&...args)
		{
			auto newEntry = ListElementPtr{InternalAdd()};

			// As the structure was malloced(), we need to initialise the shared_ptr
			// with an in-place new() and use perfect forwarding to construct the 
			// actual element value
			new(&newEntry->Item) std::shared_ptr<T>(new T(std::forward<Args>(args)...));
		}

		auto GetDepth() -> Win32::USHORT
		{
			return m_listHeader ? Win32::QueryDepthSList(m_listHeader) : 0;
		}

		void Add(std::shared_ptr<T> newVal)
		{
			auto newEntry = ListElementPtr{InternalAdd()};

			// As the structure was malloced(), we need to initialise the shared_ptr
			// with an in-place new() and use perfect forwarding to construct the 
			// actual element value
			new(&newEntry->Item) std::shared_ptr<T>(std::move(newVal));
		}

		auto Pop() -> std::shared_ptr<T>
		{
			// Using boring32error causes an internal compiler error. No idea why.
			if (not m_listHeader)
				throw Error::Boring32Error{ "Cannot pop null list header" };

			auto listEntry = Win32::PSLIST_ENTRY{Win32::InterlockedPopEntrySList(m_listHeader)};
			if (not listEntry)
				return nullptr;

			auto listItem = reinterpret_cast<ListElementPtr>(listEntry);
			auto item = std::shared_ptr<T>{ listItem->Item };
			//listItem->Item.~T();
			listItem->Item.~shared_ptr();
			_aligned_free(listEntry);
			return item;
		}

		void EmptyList()
		{
			if (not m_listHeader)
				return;

			while (Pop()) {}
			// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedflushslist
			auto entry = Win32::PSLIST_ENTRY{Win32::InterlockedFlushSList(m_listHeader)};
		}

		///		Gets and item from the list. Note that because items are 
		///		added to the front of the list, the last item added is
		///		index 0.
		/// WARNING: doesn't work. Not sure why, but EntryInfo.Next is always null.
		auto GetAt(Win32::UINT index) -> std::shared_ptr<T>
		{
			if (not m_firstEntry)
				return nullptr;
				
			auto desiredEntry = ListElementPtr{m_firstEntry};
			for (Win32::UINT i = 0; i < index; i++)
				if (desiredEntry)
					desiredEntry = reinterpret_cast<ListElementPtr>(desiredEntry->EntryInfo.Next);
				
			return desiredEntry ? desiredEntry->Item : nullptr;
		}

	private:
		auto Move(SinglyLinkedList& other) -> SinglyLinkedList&
		{
			Close();
			m_listHeader = std::exchange(other.m_listHeader, nullptr);
			m_firstEntry = std::exchange(other.m_firstEntry, nullptr);
			return *this;
		}

		auto InternalAdd() -> ListElementPtr
		{
			auto newEntry = reinterpret_cast<ListElementPtr>(
				_aligned_malloc(sizeof(ListElement<T>), Win32::MemoryAllocationAlignment));
			if (not newEntry)
				throw Error::Boring32Error("_aligned_malloc() failed");
			if (not m_firstEntry)
				m_firstEntry = newEntry;

			// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedpushentryslist
			// Note that it adds to the front of the list, not the back, which is why
			// we update the firstEntry variable
			auto addedEntry = Win32::PSLIST_ENTRY{
				Win32::InterlockedPushEntrySList(
					m_listHeader,
					&newEntry->EntryInfo
				)};
			if (addedEntry)
				m_firstEntry = newEntry;

			return newEntry;
		}

		Win32::PSLIST_HEADER m_listHeader = nullptr;
		ListElementPtr m_firstEntry = nullptr;
	};
}
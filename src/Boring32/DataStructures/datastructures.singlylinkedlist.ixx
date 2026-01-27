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

	/// <summary>
	/// See https://docs.microsoft.com/en-us/windows/win32/sync/interlocked-singly-linked-lists
	/// and https://docs.microsoft.com/en-us/windows/win32/sync/using-singly-linked-lists
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	struct SinglyLinkedList final
	{
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
				throw Error::Boring32Error("_aligned_malloc() failed");
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

		template<typename...Args>
		void Add(Args&&...args)
		{
			ListElement<T>* newEntry = InternalAdd();

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
			ListElement<T>* newEntry = InternalAdd();

			// As the structure was malloced(), we need to initialise the shared_ptr
			// with an in-place new() and use perfect forwarding to construct the 
			// actual element value
			new(&newEntry->Item) std::shared_ptr<T>(std::move(newVal));
		}

		auto Pop() -> std::shared_ptr<T>
		{
			// Using boring32error causes an internal compiler error. No idea why.
			if (not m_listHeader)
				throw Error::Boring32Error("Cannot pop null list header");

			Win32::PSLIST_ENTRY listEntry = Win32::InterlockedPopEntrySList(m_listHeader);
			if (not listEntry)
				return nullptr;

			auto listItem = reinterpret_cast<ListElement<T>*>(listEntry);
			std::shared_ptr<T> item = listItem->Item;
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
			Win32::PSLIST_ENTRY entry = Win32::InterlockedFlushSList(m_listHeader);
		}

		///		Gets and item from the list. Note that because items are 
		///		added to the front of the list, the last item added is
		///		index 0.
		/// WARNING: doesn't work. Not sure why, but EntryInfo.Next is always null.
		auto GetAt(const Win32::UINT index) -> std::shared_ptr<T>
		{
			if (not m_firstEntry)
				return nullptr;
				
			ListElement<T>* desiredEntry = m_firstEntry;
			for (Win32::UINT i = 0; i < index; i++)
				if (desiredEntry)
					desiredEntry = reinterpret_cast<ListElement<T>*>(desiredEntry->EntryInfo.Next);
				
			return desiredEntry ? desiredEntry->Item : nullptr;
		}

	private:
		auto Move(SinglyLinkedList& other) -> SinglyLinkedList&
		{
			Close();
			m_listHeader = other.m_listHeader;
			m_firstEntry = other.m_firstEntry;
			other.m_listHeader = nullptr;
			other.m_firstEntry = nullptr;
			return *this;
		}

		auto InternalAdd() -> ListElement<T>*
		{
			const auto newEntry = reinterpret_cast<ListElement<T>*>(
				_aligned_malloc(sizeof(ListElement<T>), Win32::MemoryAllocationAlignment));
			if (not newEntry)
				throw Error::Boring32Error("_aligned_malloc() failed");
			if (not m_firstEntry)
				m_firstEntry = newEntry;

			// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedpushentryslist
			// Note that it adds to the front of the list, not the back, which is why
			// we update the firstEntry variable
			Win32::PSLIST_ENTRY addedEntry = Win32::InterlockedPushEntrySList(
				m_listHeader,
				&newEntry->EntryInfo
			);
			if (addedEntry)
				m_firstEntry = newEntry;

			return newEntry;
		}

		Win32::PSLIST_HEADER m_listHeader = nullptr;
		ListElement<T>* m_firstEntry = nullptr;
	};
}
module;

#include <stdexcept>
#include <utility>
#include <source_location>
#include <memory>
#include <windows.h>
#include <malloc.h>

export module boring32.datastructures:singlylinkedlist;
import boring32.error;

export namespace Boring32::DataStructures
{
	template<typename T>
	struct ListElement 
	{
		SLIST_ENTRY EntryInfo;
		std::shared_ptr<T> Item;
	};

	/// <summary>
	/// See https://docs.microsoft.com/en-us/windows/win32/sync/interlocked-singly-linked-lists
	/// and https://docs.microsoft.com/en-us/windows/win32/sync/using-singly-linked-lists
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class SinglyLinkedList
	{
		public:
			virtual ~SinglyLinkedList()
			{
				Close();
			}

			SinglyLinkedList()
				: m_firstEntry(nullptr),
				m_listHeader(nullptr)
			{
				m_listHeader = (PSLIST_HEADER)_aligned_malloc(
					sizeof(SLIST_HEADER),
					MEMORY_ALLOCATION_ALIGNMENT
				);
				// Using boring32error causes an internal compiler error. No idea why.
				if (!m_listHeader)
					throw std::runtime_error(__FUNCSIG__ ": _aligned_malloc() failed");
				InitializeSListHead(m_listHeader);
			}

			SinglyLinkedList(const SinglyLinkedList& other) = delete;

			SinglyLinkedList(SinglyLinkedList&& other) noexcept
				: SinglyLinkedList()
			{
				Move(other);
			}

		public:
			virtual SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;
			virtual SinglyLinkedList& operator=(SinglyLinkedList&& other) noexcept
			{
				return Move(other);
			}

		public:
			virtual void Close()
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

			virtual USHORT GetDepth()
			{
				if (m_listHeader == nullptr)
					return 0;
				return QueryDepthSList(m_listHeader);
			}

			virtual void Add(std::shared_ptr<T> newVal)
			{
				ListElement<T>* newEntry = InternalAdd();

				// As the structure was malloced(), we need to initialise the shared_ptr
				// with an in-place new() and use perfect forwarding to construct the 
				// actual element value
				new(&newEntry->Item) std::shared_ptr<T>(std::move(newVal));
			}

			virtual std::shared_ptr<T> Pop()
			{
				// Using boring32error causes an internal compiler error. No idea why.
				if (!m_listHeader)
					throw std::runtime_error("Cannot pop null list header");

				PSLIST_ENTRY listEntry = InterlockedPopEntrySList(m_listHeader);
				if (listEntry == nullptr)
					return nullptr;

				auto listItem = (ListElement<T>*)listEntry;
				std::shared_ptr<T> item = listItem->Item;
				//listItem->Item.~T();
				listItem->Item.~shared_ptr();
				_aligned_free(listEntry);
				return item;
			}

			virtual void EmptyList()
			{
				if (m_listHeader)
				{
					while (Pop() != nullptr)
						;
					// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedflushslist
					PSLIST_ENTRY entry = InterlockedFlushSList(
						m_listHeader
					);
				}
			}

			/// <summary>
			///		Gets and item from the list. Note that because items are 
			///		added to the front of the list, the last item added is
			///		index 0.
			/// </summary>
			/// <param name="index"></param>
			/// <returns></returns>
			virtual std::shared_ptr<T> GetAt(const UINT index)
			{
				if (m_firstEntry == nullptr)
					return nullptr;
				
				ListElement<T>* desiredEntry = m_firstEntry;
				for (UINT i = 0; i < index; i++)
					if (desiredEntry != nullptr)
						desiredEntry = (ListElement<T>*)desiredEntry->EntryInfo.Next;
				
				return desiredEntry ? desiredEntry->Item : nullptr;
			}

		protected:
			virtual SinglyLinkedList& Move(SinglyLinkedList& other)
			{
				Close();
				m_listHeader = other.m_listHeader;
				m_firstEntry = other.m_firstEntry;
				other.m_listHeader = nullptr;
				other.m_firstEntry = nullptr;
				return *this;
			}

			virtual ListElement<T>* InternalAdd()
			{
				const auto newEntry = (ListElement<T>*)_aligned_malloc(
					sizeof(ListElement<T>),
					MEMORY_ALLOCATION_ALIGNMENT
				);
				if (newEntry == nullptr)
					throw std::runtime_error(__FUNCSIG__ ": _aligned_malloc() failed");
				if (m_firstEntry == nullptr)
					m_firstEntry = newEntry;

				// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedpushentryslist
				// Note that it adds to the front of the list, not the back, which is why
				// we update the firstEntry variable
				PSLIST_ENTRY addedEntry = InterlockedPushEntrySList(
					m_listHeader,
					&newEntry->EntryInfo
				);
				if (addedEntry != nullptr)
					m_firstEntry = (ListElement<T>*) addedEntry;

				return newEntry;
			}

		protected:
			PSLIST_HEADER m_listHeader;
			ListElement<T>* m_firstEntry;
	};
}
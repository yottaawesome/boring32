module;

#include <stdexcept>
#include <utility>
#include <memory>
#include <windows.h>
#include <malloc.h>

export module boring32.datastructures.singlylinkedlist;

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
				: firstEntry(nullptr),
				listHeader(nullptr)
			{
				listHeader = (PSLIST_HEADER)_aligned_malloc(
					sizeof(SLIST_HEADER),
					MEMORY_ALLOCATION_ALIGNMENT
				);
				if (listHeader == nullptr)
					throw std::runtime_error(__FUNCSIG__ ": _aligned_malloc() failed");
				InitializeSListHead(listHeader);
			}

		public:
			virtual void Close()
			{
				if (listHeader)
				{
					EmptyList();
					_aligned_free(listHeader);
					listHeader = nullptr;
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
				if (listHeader == nullptr)
					return 0;
				return QueryDepthSList(listHeader);
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
				PSLIST_ENTRY listEntry = InterlockedPopEntrySList(listHeader);
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
				if (listHeader)
				{
					while (Pop() != nullptr)
						;
					PSLIST_ENTRY entry = InterlockedFlushSList(
						listHeader
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
				if (firstEntry == nullptr)
					return nullptr;
				
				ListElement<T>* desiredEntry = firstEntry;
				for (UINT i = 0; i < index; i++)
					if (desiredEntry != nullptr)
						desiredEntry = (ListElement<T>*)desiredEntry->EntryInfo.Next;
				
				return desiredEntry ? desiredEntry->Item : nullptr;
			}

		protected:
			virtual ListElement<T>* InternalAdd()
			{
				const auto newEntry = (ListElement<T>*)_aligned_malloc(
					sizeof(ListElement<T>),
					MEMORY_ALLOCATION_ALIGNMENT
				);
				if (newEntry == nullptr)
					throw std::runtime_error(__FUNCSIG__ ": _aligned_malloc() failed");
				if (firstEntry == nullptr)
					firstEntry = newEntry;

				// https://docs.microsoft.com/en-us/windows/win32/api/interlockedapi/nf-interlockedapi-interlockedpushentryslist
				// Note that it adds to the front of the list, not the back, which is why
				// we update the firstEntry variable
				PSLIST_ENTRY addedEntry = InterlockedPushEntrySList(
					listHeader,
					&newEntry->EntryInfo
				);
				if (addedEntry != nullptr)
					firstEntry = (ListElement<T>*) addedEntry;

				return newEntry;
			}

		protected:
			PSLIST_HEADER listHeader;
			ListElement<T>* firstEntry;
	};
}
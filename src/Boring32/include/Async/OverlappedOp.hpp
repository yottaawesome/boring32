#pragma once
#include "Event.hpp"

namespace Boring32::Async
{
	class OverlappedOp
	{
		public:
			virtual ~OverlappedOp();
			OverlappedOp();
			OverlappedOp(
				const bool isInheritable,
				const std::wstring name
			);
			
		// Non-copyable, moveable
		public:
			OverlappedOp(const OverlappedOp& other) = delete;
			virtual void operator=(const OverlappedOp& other) = delete;	
			OverlappedOp(OverlappedOp&& other) noexcept;
			virtual void operator=(OverlappedOp&& other) noexcept;

		public:
			virtual void WaitForCompletion(const DWORD timeout);
			virtual OVERLAPPED* GetOverlapped();
			virtual uint64_t GetStatus();
			virtual uint64_t GetBytesTransferred();
			virtual bool IsComplete();
			virtual bool IsSuccessful();
			virtual void Cancel();
			virtual bool Cancel(std::nothrow_t);

		public:
			Event IoEvent;
			bool CallReturnValue;
			DWORD LastErrorValue;
			Raii::Win32Handle IoHandle;

		protected:
			OVERLAPPED* IoOverlapped;
			virtual void Move(OverlappedOp& other) noexcept;
	};
}

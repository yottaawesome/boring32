#pragma once
#include <memory>
#include "Event.hpp"

namespace Boring32::Async
{
	class OverlappedOp
	{
		public:
			virtual ~OverlappedOp();
			OverlappedOp();
			OverlappedOp(const Raii::Win32Handle& handle);
			
		// Non-copyable, moveable
		public:
			OverlappedOp(const OverlappedOp& other) = delete;
			virtual OverlappedOp& operator=(const OverlappedOp& other) = delete;
			OverlappedOp(OverlappedOp&& other) noexcept;
			virtual OverlappedOp& operator=(OverlappedOp&& other) noexcept;

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
			bool CallReturnValue;
			DWORD LastErrorValue;

		protected:
			virtual void Move(OverlappedOp& other) noexcept;

		protected:
			Event m_ioEvent;
			Raii::Win32Handle m_ioHandle;
			std::unique_ptr<OVERLAPPED> m_ioOverlapped;
	};
}

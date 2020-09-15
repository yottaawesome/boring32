#pragma once
#include <memory>
#include "Event.hpp"

namespace Boring32::Async
{
	class 
		[[nodiscard("This object must remain live while the I/O operation is in progress")]] 
		OverlappedOp
	{
		public:
			virtual ~OverlappedOp();
			OverlappedOp();
			OverlappedOp(const Raii::Win32Handle& handle);
			
		// Shareable, moveable
		public:
			OverlappedOp(const OverlappedOp& other);
			virtual OverlappedOp& operator=(const OverlappedOp& other);
			OverlappedOp(OverlappedOp&& other) noexcept;
			virtual OverlappedOp& operator=(OverlappedOp&& other) noexcept;

		public:
			virtual void WaitForCompletion(const DWORD timeout);
			virtual HANDLE GetWaitableHandle() const;
			virtual OVERLAPPED* GetOverlapped();
			virtual uint64_t GetStatus() const;
			virtual uint64_t GetBytesTransferred() const;
			virtual bool IsReady() const;
			virtual bool IsComplete() const;
			virtual bool IsSuccessful() const;
			virtual bool IsPartial() const;
			virtual void SetEvent(const bool signaled);
			virtual void Cancel();
			virtual bool Cancel(std::nothrow_t);
			virtual DWORD LastError();
			virtual void LastError(const DWORD lastError);

		protected:
			virtual void Move(OverlappedOp& other) noexcept;
			virtual void Share(const OverlappedOp& other);

		protected:
			Event m_ioEvent;
			Raii::Win32Handle m_ioHandle;
			std::shared_ptr<OVERLAPPED> m_ioOverlapped;
			DWORD m_lastError;
	};
}

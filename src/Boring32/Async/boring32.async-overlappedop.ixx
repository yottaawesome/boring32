export module boring32.async:overlappedop;
import :event;
import <memory>;
import <win32.hpp>;

export namespace Boring32::Async
{
	
	class OverlappedOp [[nodiscard("This object must remain live while the I/O operation is in progress")]]
	{
		public:
			virtual ~OverlappedOp() = default;
			OverlappedOp();
			
		// Shareable, moveable
		public:
			OverlappedOp(const OverlappedOp& other);
			virtual OverlappedOp& operator=(const OverlappedOp& other);
			OverlappedOp(OverlappedOp&& other) noexcept;
			virtual OverlappedOp& operator=(OverlappedOp&& other) noexcept;

		public:
			virtual bool WaitForCompletion(const DWORD timeout);
			virtual HANDLE GetWaitableHandle() const noexcept;
			virtual OVERLAPPED* GetOverlapped();
			virtual uint64_t GetStatus() const;
			virtual uint64_t GetBytesTransferred() const noexcept;
			virtual bool IsReady() const;
			virtual bool IsComplete() const;
			virtual bool IsSuccessful() const;
			virtual bool IsPartial() const;
			virtual void SetEvent(const bool signaled);
			virtual DWORD LastError() const;
			virtual void LastError(const DWORD lastError);

		protected:
			virtual void Move(OverlappedOp& other) noexcept;
			virtual void Share(const OverlappedOp& other);
			virtual void OnSuccess();

		protected:
			Event m_ioEvent{ false, true, false, L"" };
			std::shared_ptr<OVERLAPPED> m_ioOverlapped = std::make_shared<OVERLAPPED>();
			DWORD m_lastError = 0;
	};
}

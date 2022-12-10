module;

#include <vector>
#include <functional>

export module boring32.async:eventloop;
import <win32.hpp>;

export namespace Boring32::Async
{
	class EventLoop
	{
		public:
			virtual ~EventLoop();
			EventLoop();

		public:
			virtual void Close();
			virtual bool WaitOn(const DWORD millis, const bool waitAll);
			virtual void On(HANDLE handle, std::function<void()> handler);
			virtual void Erase(HANDLE handle);
			virtual size_t Size() noexcept;

		protected:
			std::vector<std::function<void()>> m_handlers;
			std::vector<HANDLE> m_events;
			CRITICAL_SECTION m_cs;
	};
}
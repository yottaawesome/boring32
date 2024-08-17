export module boring32:async_eventloop;
import boring32.shared;
import :error;
import :async_criticalsectionlock;

export namespace Boring32::Async
{
	class EventLoop
	{
		public:
			virtual ~EventLoop()
			{
				Close();
			}

			EventLoop()
			{
				Win32::InitializeCriticalSection(&m_cs);
			}

		public:
			virtual void Close()
			{
				m_handlers.clear();
				m_events.clear();
				Win32::DeleteCriticalSection(&m_cs);
			}

			virtual bool WaitOn(const Win32::DWORD millis, const bool waitAll)
			{
				if (m_events.empty())
					throw Error::Boring32Error("m_events is empty");

				CriticalSectionLock cs(m_cs);

				const Win32::DWORD result = Win32::WaitForMultipleObjectsEx(
					static_cast<Win32::DWORD>(m_events.size()),
					&m_events[0],
					waitAll,
					millis,
					true
				);
				if (result == Win32::WaitFailed)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("WaitForMultipleObjectsEx() failed", lastError);
				}
				if (result == Win32::WaitTimeout)
					return false;
				if (result >= Win32::WaitAbandoned && result <= (Win32::WaitAbandoned + m_events.size() - 1))
					throw Error::Boring32Error("A wait object was abandoned");

				if (waitAll)
				{
					// If we waited for all events to fire, then we need to fire
					// all functions. This is because WaitForMultipleObjectsEx()
					// returns only the zero index.
					for (auto& handler : m_handlers)
						handler();
				}
				else
				{
					m_handlers.at(result)();
				}

				return true;
			}

			virtual void On(Win32::HANDLE handle, std::function<void()> handler)
			{
				CriticalSectionLock cs(m_cs);
				m_events.push_back(handle);
				m_handlers.push_back(std::move(handler));
			}

			virtual void Erase(Win32::HANDLE handle)
			{
				CriticalSectionLock cs(m_cs);
				std::vector<Win32::HANDLE>::iterator handlePosIterator = std::find_if(
					m_events.begin(),
					m_events.end(),
					[handle](const auto& elem) { return elem == handle; }
				);
				if (handlePosIterator != m_events.end())
				{
					std::ptrdiff_t iteratorDist = std::distance(m_events.begin(), handlePosIterator);
					m_events.erase(handlePosIterator);
					m_handlers.erase(m_handlers.begin() + iteratorDist);
				}
			}

			virtual size_t Size() noexcept
			{
				CriticalSectionLock cs(m_cs);
				return m_events.size();
			}

		protected:
			std::vector<std::function<void()>> m_handlers;
			std::vector<Win32::HANDLE> m_events;
			Win32::CRITICAL_SECTION m_cs;
	};
}
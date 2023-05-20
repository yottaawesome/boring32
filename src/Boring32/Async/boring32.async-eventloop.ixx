module;

#include <source_location>;

export module boring32.async:eventloop;
import <vector>;
import <functional>;
import <stdexcept>;
import <algorithm>;
import <win32.hpp>;
import boring32.error;
import :criticalsectionlock;

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
				InitializeCriticalSection(&m_cs);
			}

		public:
			virtual void Close()
			{
				m_handlers.clear();
				m_events.clear();
				DeleteCriticalSection(&m_cs);
			}

			virtual bool WaitOn(const DWORD millis, const bool waitAll)
			{
				if (m_events.empty())
					throw Error::Boring32Error("m_events is empty");

				CriticalSectionLock cs(m_cs);

				const DWORD result = WaitForMultipleObjectsEx(
					static_cast<DWORD>(m_events.size()),
					&m_events[0],
					waitAll,
					millis,
					true
				);
				if (result == WAIT_FAILED)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("WaitForMultipleObjectsEx() failed", lastError);
				}
				if (result == WAIT_TIMEOUT)
					return false;
				if (result >= WAIT_ABANDONED && result <= (WAIT_ABANDONED + m_events.size() - 1))
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

			virtual void On(HANDLE handle, std::function<void()> handler)
			{
				CriticalSectionLock cs(m_cs);
				m_events.push_back(handle);
				m_handlers.push_back(std::move(handler));
			}

			virtual void Erase(HANDLE handle)
			{
				CriticalSectionLock cs(m_cs);
				std::vector<HANDLE>::iterator handlePosIterator = std::find_if(
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
			std::vector<HANDLE> m_events;
			CRITICAL_SECTION m_cs;
	};
}
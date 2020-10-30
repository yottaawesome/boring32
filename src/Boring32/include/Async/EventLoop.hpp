#pragma once
#include <vector>
#include <map>
#include <functional>
#include "../Raii/Win32Handle.hpp"

namespace Boring32::Async
{
	class EventLoop
	{
		public:
			virtual ~EventLoop();
			EventLoop();
			EventLoop(std::map<HANDLE, std::function<void()>> mapOfEvents);

		public:
			virtual void Close();
			virtual bool WaitOn(const DWORD millis, const bool waitAll);
			virtual void Set(std::map<HANDLE, std::function<void()>> mapOfEvents);

		protected:
			virtual void RebuildEvents();

		protected:
			std::map<HANDLE, std::function<void()>> m_mapOfEvents;
			std::vector<HANDLE> m_events;
	};
}
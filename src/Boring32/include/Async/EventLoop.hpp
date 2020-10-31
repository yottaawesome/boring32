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
			EventLoop(std::map<HANDLE, std::function<void(EventLoop&)>&> mapOfEvents);

		public:
			virtual void Close();
			virtual bool WaitOn(const DWORD millis, const bool waitAll);
			virtual void Set(std::map<HANDLE, std::function<void(EventLoop&)>&> mapOfEvents);
			virtual void Set(HANDLE handle, std::function<void(EventLoop&)>& function);

		protected:
			virtual void RebuildEvents();

		protected:
			std::map<HANDLE, std::function<void(EventLoop&)>&> m_mapOfEvents;
			std::vector<HANDLE> m_events;
	};
}
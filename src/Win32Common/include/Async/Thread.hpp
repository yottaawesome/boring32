#pragma once
#include <Windows.h>
#include <functional>

namespace Win32Utils::Async
{
	enum class ThreadStatus
	{
		Finished = 0,
		Failure = 1,
		Ready = 2,
		Running = 3,
		Suspended = 4,
		Terminated = 5,
		FinishedWithError = 0,
	};

	class Thread
	{
		public:
			Thread(void* param, bool destroyOnCompletion);
			virtual ~Thread();
			virtual void Start();
			virtual void Start(int(*simpleFunc)());
			virtual void Start(std::function<int()>* func);
			virtual void Start(std::function<int()> func);
			virtual ThreadStatus GetStatus();
			virtual UINT GetReturnCode();
			virtual void Terminate();
			virtual void Suspend();
			virtual void Resume();

		protected:
			virtual UINT Run();
			virtual void Cleanup();
			static UINT WINAPI ThreadProc(void* param);

		protected:
			ThreadStatus m_status;
			UINT m_returnCode;
			UINT m_threadId;
			HANDLE m_hThread;
			bool m_destroyOnCompletion;
			void* m_threadParam;
			std::function<int()> m_func;
	};
}
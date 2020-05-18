#pragma once
#include <Windows.h>
#include <functional>
#include "../Raii/Win32Handle.hpp"
#include "ThreadStatus.hpp"

namespace Boring32::Async
{
	class Thread
	{
		public:
			virtual void Close();
			virtual ~Thread();
			Thread(void* param, bool destroyOnCompletion);

			Thread(const Thread& other);
			virtual void operator=(const Thread& other);
			virtual void Copy(const Thread& other);

			Thread(const Thread&& other) noexcept;
			virtual void operator=(Thread&& other) noexcept;
			virtual void Copy(Thread& other) noexcept;

			virtual void Start();
			virtual void Start(int(*simpleFunc)());
			virtual void Start(const std::function<int()>& func);

			virtual ThreadStatus GetStatus();
			virtual UINT GetReturnCode();

			/// <summary>
			///		Terminates the thread. Be careful when using this
			///		function, as it prevents proper clean up of the 
			///		thread's objects and may leave shared objects in  
			///		an inconsistent state. Note also that if a thread
			///		is waiting on a kernel object, it will not be 
			///		terminated until the wait is finished.
			/// </summary>
			virtual void Terminate();
			
			virtual void Suspend();
			virtual void Resume();

		protected:
			virtual UINT Run();
			static UINT WINAPI ThreadProc(void* param);

		protected:
			ThreadStatus m_status;
			UINT m_returnCode;
			UINT m_threadId;
			Raii::Win32Handle m_hThread;
			bool m_destroyOnCompletion;
			void* m_threadParam;
			std::function<int()> m_func;
	};
}
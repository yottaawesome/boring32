module;

#include <functional>
#include <Windows.h>

export module boring32.async.thread;
import boring32.raii;
import boring32.async.event;

export namespace Boring32::Async
{
	enum class ThreadStatus
	{
		Finished = 0,
		Ready = 1,
		Running = 2,
		Suspended = 3,
		Terminated = 4
	};

	class Thread
	{
		// Default constructible, movable, not copyable
		public:
			virtual ~Thread();

			Thread();
			Thread(void* param);
			Thread(Thread&& other) noexcept;
			Thread(const Thread&) = delete;

		public:
			virtual Thread& operator=(const Thread&) = delete;
			virtual Thread& operator=(Thread&& other) noexcept;
			virtual bool operator==(const ThreadStatus status) const noexcept;

		public:
			/// <summary>
			///		Terminates the thread. Be careful when using this
			///		function, as it prevents proper clean up of the 
			///		thread's objects and may leave shared objects in  
			///		an inconsistent state. Note also that if a thread
			///		is waiting on a kernel object, it will not be 
			///		terminated until the wait is finished.
			/// </summary>
			virtual void Terminate(const DWORD exitCode);
			virtual void Suspend();
			virtual void Resume();
			virtual bool Join(const DWORD waitTime);
			virtual void Close();
			virtual void Start();
			virtual void Start(int(*simpleFunc)(void*));
			virtual void Start(const std::function<int(void*)>& func);
			virtual ThreadStatus GetStatus() const noexcept;
			virtual UINT GetExitCode() const;
			virtual Raii::Win32Handle GetHandle() noexcept;
			virtual bool WaitToStart(const DWORD millis);

		protected:
			virtual UINT Run();
			virtual void Copy(const Thread& other);
			virtual void Move(Thread& other) noexcept;
			virtual void InternalStart();
			static UINT WINAPI ThreadProc(void* param);

		protected:
			ThreadStatus m_status;
			Raii::Win32Handle m_threadHandle;
			void* m_threadParam;
			std::function<int(void*)> m_func;
			Event m_started;
	};
}
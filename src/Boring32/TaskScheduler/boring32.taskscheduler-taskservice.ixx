module;

#include <string>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

export module boring32.taskscheduler:taskservice;
import :taskfolder;

export namespace Boring32::TaskScheduler
{
	class TaskService
	{
		public:
			virtual ~TaskService();

			TaskService();

		public:
			virtual void Connect();
			virtual bool Connect(const std::nothrow_t&) noexcept;
			virtual void Close() noexcept;
			virtual TaskFolder GetFolder(const std::wstring& path);

		protected:
			Microsoft::WRL::ComPtr<ITaskService> m_taskService;
	};
}
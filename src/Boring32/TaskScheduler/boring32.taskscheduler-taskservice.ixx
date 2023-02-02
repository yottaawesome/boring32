export module boring32.taskscheduler:taskservice;
import :taskfolder;
import <string>;
import <win32.hpp>;

export namespace Boring32::TaskScheduler
{
	class TaskService
	{
		public:
			virtual ~TaskService() = default;
			TaskService() = default;

		public:
			virtual operator bool() const noexcept;

		public:
			virtual void Connect();
			virtual bool Connect(const std::nothrow_t&) noexcept;
			virtual void Close() noexcept;
			virtual TaskFolder GetFolder(const std::wstring& path);
			virtual Microsoft::WRL::ComPtr<ITaskService> Get() const noexcept;

		protected:
			Microsoft::WRL::ComPtr<ITaskService> m_taskService;
	};
}
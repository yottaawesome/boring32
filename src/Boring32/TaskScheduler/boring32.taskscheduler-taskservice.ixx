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
			TaskService(const TaskService&) = default;
			TaskService(TaskService&&) = default;

		public:
			virtual operator bool() const noexcept;
			virtual bool operator==(const TaskService& other) const noexcept;

		public:
			virtual void Connect();
			virtual bool Connect(const std::nothrow_t&) noexcept;
			virtual void Close() noexcept;
			virtual TaskFolder GetRootFolder();
			virtual TaskFolder GetFolder(const std::wstring& path);
			virtual Microsoft::WRL::ComPtr<ITaskService> Get() const noexcept;

		protected:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskservice
			Microsoft::WRL::ComPtr<ITaskService> m_taskService;
	};
}
export module boring32.taskscheduler:taskfolder;
import :registeredtask;
import <string>;
import <vector>;
import <optional>;
import <win32.hpp>;

export namespace Boring32::TaskScheduler
{
	class TaskFolder
	{
		public:
			virtual ~TaskFolder() = default;
			TaskFolder() = default;
			TaskFolder(const TaskFolder&) = default;
			TaskFolder(TaskFolder&&) noexcept = default;
			TaskFolder(Microsoft::WRL::ComPtr<ITaskFolder> taskFolder);

		public:
			virtual void Close() noexcept;
			virtual std::vector<RegisteredTask> GetTasks();
			virtual std::optional<RegisteredTask> GetTask(const std::wstring& name);
			virtual void SaveOrUpdate(
				const RegisteredTask& task,
				const TASK_LOGON_TYPE logonType
			);

		protected:
			Microsoft::WRL::ComPtr<ITaskFolder> m_taskFolder;
	};
}
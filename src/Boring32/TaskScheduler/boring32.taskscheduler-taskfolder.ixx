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
			virtual operator bool() const noexcept;
			virtual bool operator==(const TaskFolder& other) const noexcept;

		public:
			virtual void Close() noexcept;
			virtual std::vector<RegisteredTask> GetTasks();
			virtual std::optional<RegisteredTask> GetTask(const std::wstring& name);
			virtual void SaveOrUpdate(
				const RegisteredTask& task,
				const TASK_LOGON_TYPE logonType
			);
			virtual std::wstring GetName() const;

		protected:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskfolder
			Microsoft::WRL::ComPtr<ITaskFolder> m_taskFolder;
	};
}
module;

#include <string>
#include <vector>
#include <optional>
#include <windows.h>
#include <taskschd.h>
#include <wrl/client.h>

export module boring32.taskscheduler.taskfolder;
import boring32.taskscheduler.registeredtask;

export namespace Boring32::TaskScheduler
{
	class TaskFolder
	{
		public:
			virtual ~TaskFolder();

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
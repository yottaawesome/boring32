export module boring32.taskscheduler:taskfolder;
import boring32.shared;
import boring32.error;
import :registeredtask;

namespace Boring32::TaskScheduler
{
	export class TaskFolder
	{
		public:
			virtual ~TaskFolder() = default;
			TaskFolder() = default;
			TaskFolder(const TaskFolder&) = default;
			TaskFolder(TaskFolder&&) noexcept = default;
			TaskFolder(Win32::ComPtr<Win32::ITaskFolder> taskFolder)
				: m_taskFolder(std::move(taskFolder))
			{ }

		public:
			virtual operator bool() const noexcept
			{
				return m_taskFolder != nullptr;
			}

			virtual bool operator==(const TaskFolder& other) const noexcept
			{
				return m_taskFolder == other.m_taskFolder;
			}

		public:
			virtual void Close() noexcept
			{
				m_taskFolder = nullptr;
			}

			virtual std::vector<RegisteredTask> GetTasks()
			{
				if (m_taskFolder == nullptr)
					throw Error::Boring32Error("m_taskFolder is nullptr");

				Win32::ComPtr<Win32::IRegisteredTaskCollection> collection;
				if (Win32::HRESULT hr = m_taskFolder->GetTasks(0, &collection); Win32::HrFailed(hr))
					throw Error::COMError("Failed to acquire tasks", hr);

				Win32::LONG count = 0;
				if (Win32::HRESULT hr = collection->get_Count(&count); Win32::HrFailed(hr))
					throw Error::COMError("Failed to acquire task count", hr);

				if (count == 0)
					return {};

				std::vector<RegisteredTask> results;
				// Collection counts are 1-indexed
				for (Win32::LONG i = 1; i <= count; i++)
				{
					Win32::ComPtr<Win32::IRegisteredTask> task;
					if (Win32::HRESULT hr = collection->get_Item(Win32::_variant_t(i), &task); Win32::HrFailed(hr))
						throw Error::COMError("Failed to get task item", hr);

					results.push_back(task);
				}

				return results;
			}

			virtual std::optional<RegisteredTask> GetTask(const std::wstring& name)
			{
				std::vector<RegisteredTask> tasks = GetTasks();
				for (const auto& task : tasks)
					if (task.GetName() == name)
						return task;
				return std::nullopt;
			}

			virtual void SaveOrUpdate(
				const RegisteredTask& task,
				const Win32::TASK_LOGON_TYPE logonType
			)
			{
				if (!m_taskFolder)
					throw Error::Boring32Error("m_taskFolder is null");

				Win32::ComPtr<Win32::IRegisteredTask> registeredTask;
				const HRESULT hr = m_taskFolder->RegisterTaskDefinition(
					Win32::_bstr_t(task.GetName().c_str()),
					task.GetTaskDefinition().Get(),
					Win32::TASK_CREATION::TASK_CREATE_OR_UPDATE,
					Win32::_variant_t(),
					Win32::_variant_t(),
					logonType,
					Win32::_variant_t(L""),
					&registeredTask
				);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to save or update task", hr);
			}

			virtual std::wstring GetName() const
			{
				if (!m_taskFolder)
					throw Error::Boring32Error("m_taskFolder is null");

				Win32::_bstr_t name;
				const HRESULT hr = m_taskFolder->get_Name(name.GetAddress());
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get TaskFolder name", hr);

				return { name, name.length() };
			}

			virtual std::wstring GetPath() const
			{
				if (!m_taskFolder)
					throw Error::Boring32Error("m_taskFolder is null");

				Win32::_bstr_t path;
				const Win32::HRESULT hr = m_taskFolder->get_Path(path.GetAddress());
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get TaskFolder path", hr);

				return { path, path.length() };
			}

			virtual void DeleteFolder(const std::wstring& folderPath)
			{
				if (!m_taskFolder)
					throw Error::Boring32Error("m_taskFolder is null");
				if (folderPath.empty())
					throw Error::Boring32Error("folderPath is empty");

				const Win32::HRESULT hr = m_taskFolder->DeleteFolder(
					Win32::_bstr_t(folderPath.c_str()),
					0
				);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to delete task folder", hr);
			}

			virtual std::wstring GetDACL() const
			{
				if (!m_taskFolder)
					throw Error::Boring32Error("m_taskFolder is null");

				Win32::_bstr_t sddl;
				const HRESULT hr = m_taskFolder->GetSecurityDescriptor(
					Win32::_DACL_SECURITY_INFORMATION, // https://learn.microsoft.com/en-us/windows/win32/secauthz/security-information
					sddl.GetAddress()
				);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get task folder DACL", hr);

				return { sddl, sddl.length() };
			}

		protected:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskfolder
			Win32::ComPtr<Win32::ITaskFolder> m_taskFolder;
	};
}
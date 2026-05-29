export module boring32:taskscheduler.taskfolder;
import std;
import :win32;
import :error;
import :taskscheduler.registeredtask;

export namespace Boring32::TaskScheduler
{
	class TaskFolder final
	{
	public:
		TaskFolder() = default;
		TaskFolder(Win32::ComPtr<Win32::ITaskFolder> taskFolder)
			: m_taskFolder(std::move(taskFolder))
		{ }

		operator bool() const noexcept
		{
			return m_taskFolder != nullptr;
		}

		auto operator==(const TaskFolder& other) const noexcept -> bool
		{
			return m_taskFolder == other.m_taskFolder;
		}

		void Close() noexcept
		{
			m_taskFolder = nullptr;
		}

		auto GetTasks() -> std::vector<RegisteredTask>
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is nullptr" };

			auto collection = Win32::ComPtr<Win32::IRegisteredTaskCollection>{};
			if (Win32::HRESULT hr = m_taskFolder->GetTasks(0, &collection); Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to acquire tasks"};

			auto count = Win32::LONG{};
			if (Win32::HRESULT hr = collection->get_Count(&count); Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to acquire task count"};

			if (count == 0)
				return {};

			auto results = std::vector<RegisteredTask>{};
			// Collection counts are 1-indexed
			for (auto i = Win32::LONG{ 1 }; i <= count; i++)
			{
				auto task = Win32::ComPtr<Win32::IRegisteredTask>{};
				if (Win32::HRESULT hr = collection->get_Item(Win32::_variant_t(i), &task); Win32::HrFailed(hr))
					throw Error::COMError{hr, "Failed to get task item"};

				results.push_back(task);
			}

			return results;
		}

		auto GetTask(const std::wstring& name) -> std::optional<RegisteredTask>
		{
			auto tasks = std::vector<RegisteredTask>{ GetTasks() };
			for (const auto& task : tasks)
				if (task.GetName() == name)
					return task;
			return std::nullopt;
		}

		void SaveOrUpdate(const RegisteredTask& task, Win32::TASK_LOGON_TYPE logonType)
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is null" };

			auto registeredTask = Win32::ComPtr<Win32::IRegisteredTask>{};
			auto hr = m_taskFolder->RegisterTaskDefinition(
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
				throw Error::COMError{hr, "Failed to save or update task"};
		}

		auto GetName() const -> std::wstring
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is null" };

			auto name = Win32::_bstr_t{};
			auto hr = m_taskFolder->get_Name(name.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to get TaskFolder name"};

			return { name, name.length() };
		}

		auto GetPath() const -> std::wstring
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is null" };

			auto path = Win32::_bstr_t{};
			auto hr = m_taskFolder->get_Path(path.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to get TaskFolder path"};

			return { path, path.length() };
		}

		void DeleteFolder(const std::wstring& folderPath)
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is null" };
			if (folderPath.empty())
				throw Error::Boring32Error{ "folderPath is empty" };

			auto hr = m_taskFolder->DeleteFolder(Win32::_bstr_t(folderPath.c_str()), 0);
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to delete task folder"};
		}

		auto GetDACL() const -> std::wstring
		{
			if (not m_taskFolder)
				throw Error::Boring32Error{ "m_taskFolder is null" };

			auto sddl = Win32::_bstr_t{};
			// https://learn.microsoft.com/en-us/windows/win32/secauthz/security-information
			auto hr = m_taskFolder->GetSecurityDescriptor(Win32::DaclSecurityInformation, sddl.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError{hr, "Failed to get task folder DACL"};

			return { sddl, sddl.length() };
		}

	private:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskfolder
		Win32::ComPtr<Win32::ITaskFolder> m_taskFolder;
	};
}

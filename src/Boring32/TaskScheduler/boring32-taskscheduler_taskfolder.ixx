export module boring32:taskscheduler_taskfolder;
import std;
import boring32.win32;
import :error;
import :taskscheduler_registeredtask;

namespace Boring32::TaskScheduler
{
	export struct TaskFolder final
	{
		TaskFolder() = default;
		TaskFolder(Win32::ComPtr<Win32::ITaskFolder> taskFolder)
			: m_taskFolder(std::move(taskFolder))
		{ }

		operator bool() const noexcept
		{
			return m_taskFolder != nullptr;
		}

		bool operator==(const TaskFolder& other) const noexcept
		{
			return m_taskFolder == other.m_taskFolder;
		}

		void Close() noexcept
		{
			m_taskFolder = nullptr;
		}

		std::vector<RegisteredTask> GetTasks()
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is nullptr");

			Win32::ComPtr<Win32::IRegisteredTaskCollection> collection;
			if (Win32::HRESULT hr = m_taskFolder->GetTasks(0, &collection); Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to acquire tasks");

			Win32::LONG count = 0;
			if (Win32::HRESULT hr = collection->get_Count(&count); Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to acquire task count");

			if (count == 0)
				return {};

			std::vector<RegisteredTask> results;
			// Collection counts are 1-indexed
			for (Win32::LONG i = 1; i <= count; i++)
			{
				Win32::ComPtr<Win32::IRegisteredTask> task;
				if (Win32::HRESULT hr = collection->get_Item(Win32::_variant_t(i), &task); Win32::HrFailed(hr))
					throw Error::COMError(hr, "Failed to get task item");

				results.push_back(task);
			}

			return results;
		}

		std::optional<RegisteredTask> GetTask(const std::wstring& name)
		{
			std::vector<RegisteredTask> tasks = GetTasks();
			for (const auto& task : tasks)
				if (task.GetName() == name)
					return task;
			return std::nullopt;
		}

		void SaveOrUpdate(const RegisteredTask& task, Win32::TASK_LOGON_TYPE logonType)
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is null");

			Win32::ComPtr<Win32::IRegisteredTask> registeredTask;
			HRESULT hr = m_taskFolder->RegisterTaskDefinition(
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
				throw Error::COMError(hr, "Failed to save or update task");
		}

		std::wstring GetName() const
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is null");

			Win32::_bstr_t name;
			HRESULT hr = m_taskFolder->get_Name(name.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to get TaskFolder name");

			return { name, name.length() };
		}

		std::wstring GetPath() const
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is null");

			Win32::_bstr_t path;
			Win32::HRESULT hr = m_taskFolder->get_Path(path.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to get TaskFolder path");

			return { path, path.length() };
		}

		void DeleteFolder(const std::wstring& folderPath)
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is null");
			if (folderPath.empty())
				throw Error::Boring32Error("folderPath is empty");

			Win32::HRESULT hr = m_taskFolder->DeleteFolder(Win32::_bstr_t(folderPath.c_str()), 0);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to delete task folder");
		}

		std::wstring GetDACL() const
		{
			if (not m_taskFolder)
				throw Error::Boring32Error("m_taskFolder is null");

			Win32::_bstr_t sddl;
			// https://learn.microsoft.com/en-us/windows/win32/secauthz/security-information
			HRESULT hr = m_taskFolder->GetSecurityDescriptor(Win32::DaclSecurityInformation, sddl.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to get task folder DACL");

			return { sddl, sddl.length() };
		}

		private:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskfolder
		Win32::ComPtr<Win32::ITaskFolder> m_taskFolder;
	};
}
module boring32.taskscheduler:taskfolder;
import boring32.error;
import <stdexcept>;

namespace Boring32::TaskScheduler
{
	using Microsoft::WRL::ComPtr;

	TaskFolder::TaskFolder(ComPtr<ITaskFolder> taskFolder)
	:	m_taskFolder(std::move(taskFolder))
	{ }

	TaskFolder::operator bool() const noexcept
	{
		return m_taskFolder != nullptr;
	}

	bool TaskFolder::operator==(const TaskFolder& other) const noexcept
	{
		return m_taskFolder == other.m_taskFolder;
	}

	void TaskFolder::Close() noexcept
	{
		m_taskFolder = nullptr;
	}

	std::vector<RegisteredTask> TaskFolder::GetTasks()
	{
		if (m_taskFolder == nullptr)
			throw Error::Boring32Error("m_taskFolder is nullptr");

		ComPtr<IRegisteredTaskCollection> collection;
		if (HRESULT hr = m_taskFolder->GetTasks(0, &collection); FAILED(hr))
			throw Error::COMError("Failed to acquire tasks", hr);

		LONG count = 0;
		if (HRESULT hr = collection->get_Count(&count); FAILED(hr))
			throw Error::COMError("Failed to acquire task count", hr);

		if (count == 0)
			return {};

		std::vector<RegisteredTask> results;
		// Collection counts are 1-indexed
		for (LONG i = 1; i <= count; i++)
		{
			ComPtr<IRegisteredTask> task;
			if (HRESULT hr = collection->get_Item(_variant_t(i), &task); FAILED(hr))
				throw Error::COMError("Failed to get task item", hr);

			results.push_back(task);
		}

		return results;
	}

	std::optional<RegisteredTask> TaskFolder::GetTask(const std::wstring& name)
	{
		std::vector<RegisteredTask> tasks = GetTasks();
		for (const auto& task : tasks)
			if (task.GetName() == name)
				return task;
		return std::nullopt;
	}

	void TaskFolder::SaveOrUpdate(
		const RegisteredTask& task,
		const TASK_LOGON_TYPE logonType
	)
	{
		if (!m_taskFolder)
			throw Error::Boring32Error("m_taskFolder is null");

		ComPtr<IRegisteredTask> registeredTask;
		const HRESULT hr = m_taskFolder->RegisterTaskDefinition(
			_bstr_t(task.GetName().c_str()),
			task.GetTaskDefinition().Get(),
			TASK_CREATE_OR_UPDATE,
			_variant_t(),
			_variant_t(),
			logonType,
			_variant_t(L""),
			&registeredTask
		);
		if (FAILED(hr))
			throw Error::COMError("Failed to save or update task", hr);
	}

	std::wstring TaskFolder::GetName() const
	{
		if (!m_taskFolder)
			throw Error::Boring32Error("m_taskFolder is null");

		bstr_t name;
		const HRESULT hr = m_taskFolder->get_Name(name.GetAddress());
		if (FAILED(hr))
			throw Error::COMError("Failed to get TaskFolder name", hr);

		return { name, name.length() };
	}

	std::wstring TaskFolder::GetPath() const
	{
		if (!m_taskFolder)
			throw Error::Boring32Error("m_taskFolder is null");

		bstr_t path;
		const HRESULT hr = m_taskFolder->get_Path(path.GetAddress());
		if (FAILED(hr))
			throw Error::COMError("Failed to get TaskFolder path", hr);

		return { path, path.length() };
	}

	void TaskFolder::DeleteFolder(const std::wstring& folderPath)
	{
		if (!m_taskFolder)
			throw Error::Boring32Error("m_taskFolder is null");
		if (folderPath.empty())
			throw Error::Boring32Error("folderPath is empty");

		const HRESULT hr = m_taskFolder->DeleteFolder(
			_bstr_t(folderPath.c_str()),
			0
		);
		if (FAILED(hr))
			throw Error::COMError("Failed to delete task folder", hr);
	}

	std::wstring TaskFolder::GetDACL() const
	{
		if (!m_taskFolder)
			throw Error::Boring32Error("m_taskFolder is null");
		
		bstr_t sddl;
		const HRESULT hr = m_taskFolder->GetSecurityDescriptor(
			DACL_SECURITY_INFORMATION, // https://learn.microsoft.com/en-us/windows/win32/secauthz/security-information
			sddl.GetAddress()
		);
		if (FAILED(hr))
			throw Error::COMError("Failed to get task folder DACL", hr);

		return { sddl, sddl.length() };
	}
}
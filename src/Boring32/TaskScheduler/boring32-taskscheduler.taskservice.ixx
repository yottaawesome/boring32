export module boring32:taskscheduler.taskservice;
import std;
import :win32;
import :error;
import :taskscheduler.taskfolder;

export namespace Boring32::TaskScheduler
{
	struct TaskService final
	{
		TaskService() = default;

		operator bool() const noexcept
		{
			return m_taskService != nullptr;
		}

		bool operator==(const TaskService& other) const noexcept
		{
			return m_taskService == other.m_taskService;
		}

		void Connect()
		{
			if (m_taskService)
				return;

			Win32::HRESULT hr = Win32::CoCreateInstance(
				Win32::CLSID_TaskScheduler,
				nullptr,
				Win32::CLSCTX::CLSCTX_INPROC_SERVER,
				Win32::IID_ITaskService,
				&m_taskService
			);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to create ITaskService");

			hr = m_taskService->Connect(
				Win32::_variant_t(),
				Win32::_variant_t(),
				Win32::_variant_t(),
				Win32::_variant_t()
			);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to connect to Task Service");
		}

		bool Connect(const std::nothrow_t&) noexcept 
		try
		{
			Connect();
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}

		void Close() noexcept
		{
			if (m_taskService)
				m_taskService = nullptr;
		}

		TaskFolder GetRootFolder()
		{
			return GetFolder(L"\\");
		}

		TaskFolder GetFolder(const std::wstring& path)
		{
			Win32::ComPtr<Win32::ITaskFolder> folder = nullptr;
			Win32::HRESULT hr = m_taskService->GetFolder(Win32::_bstr_t(path.c_str()), &folder);
			if (Win32::HrFailed(hr))
				throw Error::COMError(hr, "Failed to connect to Task Service");
			return folder;
		}

		Win32::ComPtr<Win32::ITaskService> Get() const noexcept
		{
			return m_taskService;
		}

		private:
		// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskservice
		Win32::ComPtr<Win32::ITaskService> m_taskService;
	};
}

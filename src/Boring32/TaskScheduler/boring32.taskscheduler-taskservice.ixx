export module boring32.taskscheduler:taskservice;
import :taskfolder;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import boring32.win32;

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
			virtual operator bool() const noexcept
			{
				return m_taskService != nullptr;
			}

			virtual bool operator==(const TaskService& other) const noexcept
			{
				return m_taskService == other.m_taskService;
			}

		public:
			virtual void Connect()
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
					throw Error::COMError("Failed to create ITaskService", hr);

				hr = m_taskService->Connect(
					Win32::_variant_t(),
					Win32::_variant_t(),
					Win32::_variant_t(),
					Win32::_variant_t()
				);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to connect to Task Service", hr);
			}

			virtual bool Connect(const std::nothrow_t&) noexcept try
			{
				Connect();
				return true;
			}
			catch (const std::exception&)
			{
				// ICE
				//std::wcerr << ex.what() << std::endl;
				return false;
			}

			virtual void Close() noexcept
			{
				if (m_taskService)
					m_taskService = nullptr;
			}

			virtual TaskFolder GetRootFolder()
			{
				return GetFolder(L"\\");
			}

			virtual TaskFolder GetFolder(const std::wstring& path)
			{
				Win32::ComPtr<Win32::ITaskFolder> folder = nullptr;
				Win32::HRESULT hr = m_taskService->GetFolder(Win32::_bstr_t(path.c_str()), &folder);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to connect to Task Service", hr);
				return folder;
			}

			virtual Win32::ComPtr<Win32::ITaskService> Get() const noexcept
			{
				return m_taskService;
			}

		protected:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskservice
			Win32::ComPtr<Win32::ITaskService> m_taskService;
	};
}
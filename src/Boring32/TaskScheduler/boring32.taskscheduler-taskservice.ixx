module;

#include <source_location>;

export module boring32.taskscheduler:taskservice;
import :taskfolder;
import <string>;
import <win32.hpp>;
import boring32.error;
import <iostream>;

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

				HRESULT hr = CoCreateInstance(
					CLSID_TaskScheduler,
					nullptr,
					CLSCTX_INPROC_SERVER,
					IID_ITaskService,
					&m_taskService
				);
				if (FAILED(hr))
					throw Error::COMError("Failed to create ITaskService", hr);

				hr = m_taskService->Connect(
					_variant_t(),
					_variant_t(),
					_variant_t(),
					_variant_t()
				);
				if (FAILED(hr))
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
				Microsoft::WRL::ComPtr<ITaskFolder> folder = nullptr;
				HRESULT hr = m_taskService->GetFolder(_bstr_t(path.c_str()), &folder);
				if (FAILED(hr))
					throw Error::COMError("Failed to connect to Task Service", hr);
				return folder;
			}

			virtual Microsoft::WRL::ComPtr<ITaskService> Get() const noexcept
			{
				return m_taskService;
			}

		protected:
			// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskservice
			Microsoft::WRL::ComPtr<ITaskService> m_taskService;
	};
}
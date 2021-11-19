module;

#include "pch.hpp"
#include "include/Error/Error.hpp"
#include <string>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

module boring32.taskscheduler.taskservice;

namespace Boring32::TaskScheduler
{
	TaskService::~TaskService()
	{
		Close();
	}

	TaskService::TaskService()
	{ }

	void TaskService::Connect()
	{
		if (m_taskService != nullptr)
			return;

		HRESULT hr = CoCreateInstance(
			CLSID_TaskScheduler,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_taskService)
		);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to create ITaskService", hr);
		
		hr = m_taskService->Connect(
			_variant_t(), 
			_variant_t(),
			_variant_t(), 
			_variant_t()
		);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to connect to Task Service", hr);
	}
	
	bool TaskService::Connect(const std::nothrow_t&) noexcept
	{
		return Error::TryCatchLogToWCerr([this] { Connect(); }, __FUNCSIG__);
	}

	void TaskService::Close() noexcept
	{
		if (m_taskService)
			m_taskService = nullptr;
	}

	TaskFolder TaskService::GetFolder(const std::wstring& path)
	{
		Microsoft::WRL::ComPtr<ITaskFolder> folder = nullptr;
		HRESULT hr = m_taskService->GetFolder(_bstr_t(path.c_str()), &folder);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to connect to Task Service", hr);
		return folder;
	}
}
module;

#include "pch.hpp"
#include <string>
#include <source_location>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

module boring32.taskscheduler:taskservice;
import boring32.error.functions;
import boring32.error.comerror;

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
			throw Error::ComError(std::source_location::current(), "Failed to create ITaskService", hr);
		
		hr = m_taskService->Connect(
			_variant_t(), 
			_variant_t(),
			_variant_t(), 
			_variant_t()
		);
		if (FAILED(hr))
			throw Error::ComError(std::source_location::current(), "Failed to connect to Task Service", hr);
	}
	
	bool TaskService::Connect(const std::nothrow_t&) noexcept try
	{
		Connect();
		return true;
	}
	catch (const std::exception& ex)
	{
		std::wcerr << ex.what() << std::endl;
		return false;
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
			throw Error::ComError(std::source_location::current(), "Failed to connect to Task Service", hr);
		return folder;
	}
}
#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/TaskScheduler/RegisteredTask.hpp"

namespace Boring32::TaskScheduler
{
	RegisteredTask::~RegisteredTask()
	{
		Close();
	}

	RegisteredTask::RegisteredTask(Microsoft::WRL::ComPtr<IRegisteredTask> registeredTask)
	:	m_registeredTask(std::move(registeredTask))
	{ }

	void RegisteredTask::Close() noexcept
	{
		m_registeredTask = nullptr;
	}

	std::wstring RegisteredTask::GetName() const
	{
		bstr_t taskName;
		HRESULT hr = m_registeredTask->get_Name(taskName.GetAddress());
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get Task name", hr);

		return std::wstring(taskName, taskName.length());
	}
}
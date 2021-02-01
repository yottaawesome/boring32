#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/TaskScheduler/RegisteredTask.hpp"

namespace Boring32::TaskScheduler
{
	using Microsoft::WRL::ComPtr;

	RegisteredTask::~RegisteredTask()
	{
		Close();
	}

	RegisteredTask::RegisteredTask(ComPtr<IRegisteredTask> registeredTask)
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

	void RegisteredTask::SetEnabled(const bool isEnabled)
	{
		if (m_registeredTask == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_registeredTask is nullptr");

		HRESULT hr = m_registeredTask->get_Definition(&m_taskDefinition);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get ITaskDefinition", hr);
		
		ComPtr<ITriggerCollection> triggers;
		hr = m_taskDefinition->get_Triggers(&triggers);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get trigger collection", hr);

		long count = 0;
		hr = triggers->get_Count(&count);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get trigger collection count", hr);

		for (int i = 1; i <= count; i++) // Collections start at 1
		{
			ComPtr<ITrigger> trigger = nullptr;
			hr = triggers->get_Item(i, &trigger);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);

			trigger->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
		}
	}

	Microsoft::WRL::ComPtr<IRegisteredTask> RegisteredTask::GetRegisteredTask() const
	{
		return m_registeredTask;
	}

	Microsoft::WRL::ComPtr<ITaskDefinition> RegisteredTask::GetTaskDefinition() const
	{
		return m_taskDefinition;
	}
}
module;

#include <source_location>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

module boring32.taskscheduler:registeredtask;
import boring32.error;

namespace Boring32::TaskScheduler
{
	using Microsoft::WRL::ComPtr;

	RegisteredTask::~RegisteredTask()
	{
		Close();
	}

	RegisteredTask::RegisteredTask(ComPtr<IRegisteredTask> registeredTask)
	:	m_registeredTask(std::move(registeredTask))
	{
		if (m_registeredTask)
			if (HRESULT hr = m_registeredTask->get_Definition(&m_taskDefinition); FAILED(hr))
				throw Error::ComError("Failed to get ITaskDefinition", hr);
	}

	void RegisteredTask::Close() noexcept
	{
		m_registeredTask = nullptr;
		m_taskDefinition = nullptr;
	}

	std::wstring RegisteredTask::GetName() const
	{
		CheckIsValid();

		bstr_t taskName;
		if (HRESULT hr = m_registeredTask->get_Name(taskName.GetAddress()); FAILED(hr))
			throw Error::ComError("Failed to get Task name", hr);

		return std::wstring(taskName, taskName.length());
	}

	void RegisteredTask::SetEnabled(const bool isEnabled)
	{
		CheckIsValid();

		if (HRESULT hr = m_registeredTask->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE); FAILED(hr))
			throw Error::ComError("Failed to set task enabled property", hr);

		/*std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			HRESULT hr = trigger->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to set trigger enabled", hr);
		}*/
	}

	Microsoft::WRL::ComPtr<IRegisteredTask> RegisteredTask::GetRegisteredTask() const noexcept
	{
		return m_registeredTask;
	}

	Microsoft::WRL::ComPtr<ITaskDefinition> RegisteredTask::GetTaskDefinition() const noexcept
	{
		return m_taskDefinition;
	}

	void RegisteredTask::SetRepetitionInterval(const DWORD intervalMinutes)
	{
		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			ComPtr<IRepetitionPattern> pattern;
			if (HRESULT hr = trigger->get_Repetition(&pattern); FAILED(hr))
				throw Error::ComError("Failed to get task repetition pattern", hr);

			std::wstring interval = L"PT" + std::to_wstring(intervalMinutes) + L"M";
			if (HRESULT hr = pattern->put_Interval(bstr_t(interval.c_str())); FAILED(hr))
				throw Error::ComError("Failed to set trigger repetition pattern interval", hr);
		}
	}

	void RegisteredTask::Run()
	{
		CheckIsValid();

		// The call to Run() will fail if the task doesn't have AllowDemandStart 
		// set to true or the task has been disabled. We can check for these 
		// conditions, but COM does it for us and the error is descriptive, so no 
		// need to bother.
		ComPtr<IRunningTask> runningTask;
		if (HRESULT hr = m_registeredTask->Run(_variant_t(VT_NULL), &runningTask); FAILED(hr))
			throw Error::ComError("Failed to start task", hr);
	}
	
	UINT RegisteredTask::SetRandomDelay(const DWORD minutes)
	{
		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		const std::wstring delay = L"PT" + std::to_wstring(minutes) + L"M";
		UINT triggersUpdated = 0;

		for (const auto& trigger : triggers)
		{
			TASK_TRIGGER_TYPE2 type = TASK_TRIGGER_TYPE2::TASK_TRIGGER_EVENT;
			HRESULT hr = trigger->get_Type(&type);
			if (FAILED(hr))
				throw Error::ComError("Failed to get ITrigger type", hr);

			// Some, but not all, triggers support random delays, so we only
			// set the daily one for now, as that's the one of interest.
			switch (type)
			{
				case TASK_TRIGGER_TYPE2::TASK_TRIGGER_DAILY:
				{
					ComPtr<IDailyTrigger> dailyTrigger = (IDailyTrigger*)trigger.Get();
					hr = dailyTrigger->put_RandomDelay(_bstr_t(delay.c_str()));
					if (FAILED(hr))
						throw Error::ComError("Failed to set trigger random delay", hr);
					triggersUpdated++;
					break;
				}

				default: std::wcerr
					<< L"Did not set random delay for a trigger as it's not supported"
					<< std::endl;
			}
		}

		return triggersUpdated;
	}

	std::vector<ComPtr<ITrigger>> RegisteredTask::GetTriggers()
	{
		CheckIsValid();

		ComPtr<ITriggerCollection> triggers;
		if (HRESULT hr = m_taskDefinition->get_Triggers(&triggers); FAILED(hr))
			throw Error::ComError("Failed to get trigger collection", hr);

		long count = 0;
		if (HRESULT hr = triggers->get_Count(&count); FAILED(hr))
			throw Error::ComError("Failed to get trigger collection count", hr);

		std::vector<ComPtr<ITrigger>> returnVal;
		for (int i = 1; i <= count; i++) // Collections start at 1
		{
			ComPtr<ITrigger> trigger = nullptr;
			if (HRESULT hr = triggers->get_Item(i, &trigger); FAILED(hr))
				throw Error::ComError("Failed to get trigger", hr);
			returnVal.push_back(std::move(trigger));
		}
		return returnVal;
	}

	void RegisteredTask::CheckIsValid() const
	{
		if (!m_registeredTask)
			throw std::runtime_error(__FUNCSIG__ ": m_registeredTask is nullptr");
		if (!m_taskDefinition)
			throw std::runtime_error(__FUNCSIG__ ": m_taskDefinition is nullptr");
	}
}
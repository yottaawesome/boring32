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
		{
			throw std::runtime_error(__FUNCSIG__ ": m_registeredTask is nullptr");
		}

		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			HRESULT hr = trigger->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to set trigger enabled", hr);
		}
	}

	void RegisteredTask::PrintInfo()
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


			ComPtr<IRepetitionPattern> pattern;
			hr = trigger->get_Repetition(&pattern);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);
			bstr_t interval;
			hr = pattern->get_Interval(interval.GetAddress());
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);

			//std::wstring str(interval, interval.length());
			//std::wcout << str << std::endl;

			hr = pattern->put_Interval(bstr_t(L"PT120M"));
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);
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

	void RegisteredTask::SetRepetitionInterval(const DWORD intervalMinutes)
	{
		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			ComPtr<IRepetitionPattern> pattern;
			HRESULT hr = trigger->get_Repetition(&pattern);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get task repetition pattern", hr);


			std::wstring interval = L"PT" + std::to_wstring(intervalMinutes) + L"M";
			hr = pattern->put_Interval(bstr_t(interval.c_str()));
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to set trigger repetition pattern interval", hr);
		}
	}

	std::vector<ComPtr<ITrigger>> RegisteredTask::GetTriggers()
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

		std::vector<ComPtr<ITrigger>> returnVal;
		for (int i = 1; i <= count; i++) // Collections start at 1
		{
			ComPtr<ITrigger> trigger = nullptr;
			hr = triggers->get_Item(i, &trigger);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);

			returnVal.push_back(std::move(trigger));
		}
		return returnVal;
	}
}
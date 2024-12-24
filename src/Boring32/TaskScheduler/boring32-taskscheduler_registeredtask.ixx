export module boring32:taskscheduler_registeredtask;
import boring32.shared;
import :error;

namespace Boring32::TaskScheduler
{
	export struct RegisteredTask final
	{
		RegisteredTask(const RegisteredTask&) = default;
		RegisteredTask(RegisteredTask&&) noexcept = default;
		RegisteredTask(Win32::ComPtr<Win32::IRegisteredTask> registeredTask)
			: m_registeredTask(std::move(registeredTask))
		{
			if (not m_registeredTask)
				return;
			const Win32::HRESULT hr = m_registeredTask->get_Definition(&m_taskDefinition);
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to get ITaskDefinition", hr);
		}

		void Close() noexcept
		{
			m_registeredTask = nullptr;
			m_taskDefinition = nullptr;
		}

		std::wstring GetName() const
		{
			CheckIsValid();

			Win32::_bstr_t taskName;
			const Win32::HRESULT hr = m_registeredTask->get_Name(taskName.GetAddress());
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to get Task name", hr);

			return { taskName, taskName.length() };
		}

		void SetEnabled(const bool isEnabled)
		{
			CheckIsValid();

			Win32::HRESULT hr = m_registeredTask->put_Enabled(isEnabled ? Win32::VariantTrue : Win32::VariantFalse);
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to set task enabled property", hr);

			/*std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
			for (auto& trigger : triggers)
			{
				HRESULT hr = trigger->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
				if (FAILED(hr))
					throw Error::ComError(__FUNCSIG__ ": failed to set trigger enabled", hr);
			}*/
		}

		Win32::ComPtr<Win32::IRegisteredTask> GetRegisteredTask() const noexcept
		{
			return m_registeredTask;
		}

		Win32::ComPtr<Win32::ITaskDefinition> GetTaskDefinition() const noexcept
		{
			return m_taskDefinition;
		}

		void SetRepetitionInterval(const Win32::DWORD intervalMinutes)
		{
			std::vector<Win32::ComPtr<Win32::ITrigger>> triggers = GetTriggers();
			for (auto& trigger : triggers)
			{
				Win32::ComPtr<Win32::IRepetitionPattern> pattern;
				Win32::HRESULT hr = trigger->get_Repetition(&pattern);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get task repetition pattern", hr);

				std::wstring interval = std::format(L"PT{}M", intervalMinutes);
				hr = pattern->put_Interval(Win32::_bstr_t(interval.c_str()));
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to set trigger repetition pattern interval", hr);
			}
		}

		///		Runs the Scheduled Task. Note that Scheduled Tasks
		///		cannot be run when in Disabled state or if they do 
		///		not have AllowDemandStart set to true.
		void Run()
		{
			CheckIsValid();

			// The call to Run() will fail if the task doesn't have AllowDemandStart 
			// set to true or the task has been disabled. We can check for these 
			// conditions, but COM does it for us and the error is descriptive, so no 
			// need to bother.
			Win32::ComPtr<Win32::IRunningTask> runningTask;
			Win32::HRESULT hr = m_registeredTask->Run(Win32::_variant_t(Win32::VARENUM::VT_NULL), &runningTask);
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to start task", hr);
		}

		///		Set a random delay, in minutes to all supporting
		///		triggers associated with this task. This delay 
		///		is added to the start time of the trigger. 
		///		Not all trigger types support a random delay, in 
		///		which case, this function does not modify them.
		unsigned SetRandomDelay(const Win32::DWORD minutes)
		{
			std::vector<Win32::ComPtr<Win32::ITrigger>> triggers = GetTriggers();
			const std::wstring delay = std::format(L"PT{}M", minutes);
			unsigned triggersUpdated = 0;

			for (const auto& trigger : triggers)
			{
				Win32::TASK_TRIGGER_TYPE2 type = Win32::TASK_TRIGGER_TYPE2::TASK_TRIGGER_EVENT;
				Win32::HRESULT hr = trigger->get_Type(&type);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get ITrigger type", hr);

				// Some, but not all, triggers support random delays, so we only
				// set the daily one for now, as that's the one of interest.
				switch (type)
				{
					case Win32::TASK_TRIGGER_TYPE2::TASK_TRIGGER_DAILY:
					{
						Win32::ComPtr<Win32::IDailyTrigger> dailyTrigger = (Win32::IDailyTrigger*)trigger.Get();
						hr = dailyTrigger->put_RandomDelay(Win32::_bstr_t(delay.c_str()));
						if (Win32::HrFailed(hr))
							throw Error::COMError("Failed to set trigger random delay", hr);
						triggersUpdated++;
						break;
					}

					default:
						std::wcerr << L"Did not set random delay for a trigger as it's not supported\n";
				}
			}

			return triggersUpdated;
		}

		private:
		std::vector<Win32::ComPtr<Win32::ITrigger>> GetTriggers()
		{
			CheckIsValid();

			Win32::ComPtr<Win32::ITriggerCollection> triggers;
			Win32::HRESULT hr = m_taskDefinition->get_Triggers(&triggers);
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to get trigger collection", hr);

			long count = 0;
			hr = triggers->get_Count(&count);
			if (Win32::HrFailed(hr))
				throw Error::COMError("Failed to get trigger collection count", hr);

			std::vector<Win32::ComPtr<Win32::ITrigger>> returnVal;
			for (int i = 1; i <= count; i++) // Collections start at 1
			{
				Win32::ComPtr<Win32::ITrigger> trigger = nullptr;
				hr = triggers->get_Item(i, &trigger);
				if (Win32::HrFailed(hr))
					throw Error::COMError("Failed to get trigger", hr);
				returnVal.push_back(std::move(trigger));
			}
			return returnVal;
		}

		void CheckIsValid() const
		{
			if (not m_registeredTask)
				throw Error::Boring32Error("m_registeredTask is nullptr");
			if (not m_taskDefinition)
				throw Error::Boring32Error("m_taskDefinition is nullptr");
		}

		// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-iregisteredtask
		Win32::ComPtr<Win32::IRegisteredTask> m_registeredTask;
		// https://learn.microsoft.com/en-us/windows/win32/api/taskschd/nn-taskschd-itaskdefinition
		Win32::ComPtr<Win32::ITaskDefinition> m_taskDefinition;
	};
}
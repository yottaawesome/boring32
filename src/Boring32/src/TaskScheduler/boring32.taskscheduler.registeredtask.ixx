module;

#include <string>
#include <vector>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

export module boring32.taskscheduler.registeredtask;

export namespace Boring32::TaskScheduler
{
	class RegisteredTask
	{
		public:
			virtual ~RegisteredTask();

			RegisteredTask(Microsoft::WRL::ComPtr<IRegisteredTask> registeredTask);

		public:
			virtual void Close() noexcept;
			virtual std::wstring GetName() const;
			virtual void SetEnabled(const bool isEnabled);
			virtual Microsoft::WRL::ComPtr<IRegisteredTask> GetRegisteredTask() const;
			virtual Microsoft::WRL::ComPtr<ITaskDefinition> GetTaskDefinition() const;
			virtual void SetRepetitionInterval(const DWORD intervalMinutes);

			/// <summary>
			///		Runs the Scheduled Task. Note that Scheduled Tasks
			///		cannot be run when in Disabled state or if they do 
			///		not have AllowDemandStart set to true.
			/// </summary>
			/// <exception cref="runtime_error">
			///		Thrown if this instance does not have valid COM
			///		interfaces.
			/// </exception>
			/// <exception cref="ComError">
			///		Thrown when a COM operation fails.
			/// </exception>
			virtual void Run();

			/// <summary>
			///		Set a random delay, in minutes to all supporting
			///		triggers associated with this task. This delay 
			///		is added to the start time of the trigger. 
			///		Not all trigger types support a random delay, in 
			///		which case, this function does not modify them.
			/// </summary>
			/// <param name="minutes">
			///		The upper bound of the delay in minutes.
			/// </param>
			/// <exception cref="runtime_error">
			///		Thrown if this instance does not have valid COM
			///		interfaces.
			/// </exception>
			/// <exception cref="ComError">
			///		Thrown when a COM operation fails.
			/// </exception>
			/// <returns>The number of triggers updated.</returns>
			virtual UINT SetRandomDelay(const DWORD minutes);

		protected:
			virtual std::vector<Microsoft::WRL::ComPtr<ITrigger>> GetTriggers();
			virtual void CheckIsValid() const;

		protected:
			Microsoft::WRL::ComPtr<IRegisteredTask> m_registeredTask;
			Microsoft::WRL::ComPtr<ITaskDefinition> m_taskDefinition;
	};
}
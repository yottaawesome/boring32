#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <comdef.h>
#include <taskschd.h>
#include <wrl/client.h>

namespace Boring32::TaskScheduler
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

		protected:
			Microsoft::WRL::ComPtr<IRegisteredTask> m_registeredTask;
			Microsoft::WRL::ComPtr<ITaskDefinition> m_taskDefinition;
	};
}
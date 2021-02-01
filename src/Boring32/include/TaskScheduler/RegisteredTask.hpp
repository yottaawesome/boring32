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

		protected:
			Microsoft::WRL::ComPtr<IRegisteredTask> m_registeredTask;
	};
}
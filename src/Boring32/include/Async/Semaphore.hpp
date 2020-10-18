#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"
#include "Onyx32/Async/ISemaphore.hpp"

namespace Boring32::Async
{
	class Semaphore : public Onyx32::Core::Async::ISemaphore
	{
		public:
			virtual ~Semaphore();
			virtual void Close();

			Semaphore();
			Semaphore(
				std::wstring name, 
				const bool isInheritable, 
				const long initialCount, 
				const long maxCount);
			Semaphore(
				std::wstring name,
				const bool isInheritable,
				const long initialCount,
				const long maxCount,
				const DWORD desiredAccess);

			Semaphore(const Semaphore& other);
			virtual void operator=(const Semaphore& other);
			Semaphore(Semaphore&& other) noexcept;
			virtual void operator=(Semaphore&& other) noexcept;

		public:
			virtual void Release() override;
			virtual void Release(const int countToRelease) override;
			virtual bool Acquire(const DWORD millisTimeout) override;
			virtual bool Acquire(const int countToAcquire, const DWORD millisTimeout) override;
			virtual const std::wstring& GetName() const override;
			virtual int GetCurrentCount() const override;
			virtual int GetMaxCount() const override;
			virtual HANDLE GetHandle() const override;
			virtual void Free() override;

		protected:
			virtual void Copy(const Semaphore& other);
			virtual void Move(Semaphore& other) noexcept;

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_name; 
			int m_currentCount;
			int m_maxCount;
	};
}
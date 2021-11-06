#pragma once
#include <Windows.h>
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class Semaphore
	{
		public:
			virtual ~Semaphore();

			Semaphore();
			Semaphore(
				std::wstring name, 
				const bool isInheritable, 
				const ULONG initialCount, 
				const ULONG maxCount);
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
			virtual void Close();
			virtual void Release();
			virtual void Release(const int countToRelease);
			virtual bool Acquire(const DWORD millisTimeout);
			virtual bool Acquire(const int countToAcquire, const DWORD millisTimeout);
			virtual const std::wstring& GetName() const noexcept final;
			virtual int GetCurrentCount() const noexcept final;
			virtual int GetMaxCount() const noexcept final;
			virtual HANDLE GetHandle() const noexcept final;

		protected:
			virtual void Copy(const Semaphore& other);
			virtual void Move(Semaphore& other) noexcept;

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_name; 
			std::atomic<long> m_currentCount;
			long m_maxCount;
	};
}
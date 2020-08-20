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
			virtual void Close();

			Semaphore();
			Semaphore(
				const std::wstring& name, 
				const bool isInheritable, 
				const long initialCount, 
				const long maxCount);
			Semaphore(
				const std::wstring& name,
				const bool isInheritable,
				const long initialCount,
				const long maxCount,
				const DWORD desiredAccess);

			Semaphore(const Semaphore& other);
			virtual void operator=(const Semaphore& other);
			Semaphore(Semaphore&& other) noexcept;
			virtual void operator=(Semaphore&& other) noexcept;

		public:
			virtual void Release();
			virtual void Release(const int countToRelease);
			virtual bool Acquire(const DWORD millisTimeout);
			virtual bool Acquire(const int countToAcquire, const DWORD millisTimeout);
			virtual std::wstring GetName();
			virtual int GetCurrentCount();
			virtual int GetMaxCount();
			virtual HANDLE GetHandle();

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
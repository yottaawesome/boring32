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
				const bool isInheritable,
				const ULONG initialCount,
				const ULONG maxCount);
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
			Semaphore(Semaphore&& other) noexcept;

		public:
			virtual void operator=(const Semaphore& other);
			virtual void operator=(Semaphore&& other) noexcept;

		public:
			virtual void InternalCreate(
				const std::wstring& name,
				const ULONG initialCount,
				const ULONG maxCount,
				const bool isInheritable
			);
			virtual void Close();
			virtual long Release();
			virtual long Release(const long countToRelease);
			virtual bool Acquire(const DWORD millisTimeout);
			virtual bool Acquire(const long countToAcquire, const DWORD millisTimeout);
			virtual const std::wstring& GetName() const noexcept final;
			virtual long GetCurrentCount() const noexcept final;
			virtual long GetMaxCount() const noexcept final;
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
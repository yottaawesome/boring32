#pragma once
#include "../Onyx32.hpp"
#include <string>
#include <Windows.h>

namespace Onyx32::Core::Async
{
	class ONYXHEADER ISemaphore
	{
		public:
			virtual ~ISemaphore() = 0;
			virtual void Release() = 0;
			virtual void Release(const int countToRelease) = 0;
			virtual bool Acquire(const DWORD millisTimeout) = 0;
			virtual bool Acquire(const int countToAcquire, const DWORD millisTimeout) = 0;
			virtual const std::wstring& GetName() const = 0;
			virtual int GetCurrentCount() const = 0;
			virtual int GetMaxCount() const = 0;
			virtual HANDLE GetHandle() const = 0;
			virtual void Free() = 0;
	};
}
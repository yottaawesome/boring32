#pragma once
#include <Windows.h>
#include <string>

namespace Boring32::Raii
{
	class Win32Handle
	{
		public:
			virtual ~Win32Handle();
			Win32Handle();
			Win32Handle(const HANDLE handle);

			Win32Handle(const Win32Handle& otherHandle);
			virtual void operator=(const Win32Handle& other);

			Win32Handle(Win32Handle&& otherHandle) noexcept;
			virtual void operator=(Win32Handle&& other) noexcept;

		public:
			virtual void operator=(const HANDLE other);
			virtual bool operator==(const HANDLE other) const;
			virtual bool operator==(const Win32Handle& other) const;
			virtual HANDLE* operator&();

		public:
			virtual HANDLE GetHandle() const;
			virtual HANDLE& GetHandleAddress();
			virtual void Close();
			virtual HANDLE DuplicateCurrentHandle() const;
			virtual bool IsInheritable() const;
			virtual void SetInheritability(const bool isInheritable);
			virtual HANDLE Detach();

			static bool HandleIsInheritable(const HANDLE handle);
			static HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable);

		protected:
			virtual void Copy(const Win32Handle& other);
			virtual void Move(Win32Handle& other) noexcept;
			virtual void InternalClose(const bool throwOnFailure);

		protected:
			HANDLE m_handle;
	};
}
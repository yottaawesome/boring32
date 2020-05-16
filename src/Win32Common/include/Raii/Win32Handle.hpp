#pragma once
#include <Windows.h>
#include <string>

namespace Win32Utils::Raii
{
	class Win32Handle
	{
		public:
			virtual ~Win32Handle();
			Win32Handle();
			Win32Handle(const HANDLE handle, const bool inheritable);

			Win32Handle(const Win32Handle& otherHandle);
			virtual void operator=(const Win32Handle& other);

			Win32Handle(Win32Handle&& otherHandle) noexcept;
			virtual void operator=(Win32Handle&& other) noexcept;

			virtual void operator=(const bool isInheritable);
			virtual void operator=(const HANDLE other);
			virtual bool operator==(const HANDLE other) const;
			virtual bool operator==(const Win32Handle& other) const;
			virtual HANDLE* operator&();

			virtual HANDLE GetHandle() const;
			virtual HANDLE& GetHandleAddress();
			virtual void Close();
			virtual HANDLE DuplicateCurrentHandle() const;

			virtual bool IsInheritable() const;

		protected:
			virtual void Copy(const Win32Handle& other);
			virtual HANDLE DuplicatePassedHandle(const HANDLE handle, const bool isInheritable) const;

		protected:
			HANDLE m_handle;
			bool m_inheritable;
	};
}
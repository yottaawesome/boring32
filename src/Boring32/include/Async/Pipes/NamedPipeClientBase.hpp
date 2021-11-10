#pragma once
#include <string>

import boring32.raii.win32handle;

namespace Boring32::Async
{
	class NamedPipeClientBase
	{
		public:
			virtual ~NamedPipeClientBase();
			NamedPipeClientBase();
			NamedPipeClientBase(const std::wstring& name, const DWORD fileAttributes);

		// Moveable, copyable
		public:
			NamedPipeClientBase(const NamedPipeClientBase& other);
			virtual void operator=(const NamedPipeClientBase& other);
			NamedPipeClientBase(NamedPipeClientBase&& other) noexcept;
			virtual void operator=(NamedPipeClientBase&& other) noexcept;

		public:
			virtual void SetMode(const DWORD pipeMode);
			virtual void Connect(const DWORD timeout);
			virtual bool Connect(const DWORD timeout, std::nothrow_t);
			virtual void Close();
			virtual DWORD UnreadCharactersRemaining() const;
			virtual void Flush();
			virtual void CancelCurrentThreadIo();
			virtual bool CancelCurrentThreadIo(std::nothrow_t)  noexcept;
			virtual void CancelCurrentProcessIo(OVERLAPPED* overlapped);
			virtual bool CancelCurrentProcessIo(OVERLAPPED* overlapped, std::nothrow_t) noexcept;

		protected:
			virtual void Copy(const NamedPipeClientBase& other);
			virtual void Move(NamedPipeClientBase& other) noexcept;

		protected:
			Raii::Win32Handle m_handle;
			std::wstring m_pipeName;
			DWORD m_fileAttributes;
	};
}
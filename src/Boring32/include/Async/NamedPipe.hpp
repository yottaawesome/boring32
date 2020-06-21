#pragma once
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class NamedPipe
	{
		public:
			virtual ~NamedPipe();
			NamedPipe(const std::wstring& pipeName, const DWORD size, const bool isOverlapped);

			NamedPipe(const NamedPipe& other);
			virtual void operator=(const NamedPipe& other);

			NamedPipe(NamedPipe&& other) noexcept;
			virtual void operator=(NamedPipe&& other) noexcept;

		public:
			virtual void Close();
			virtual void Connect();
			virtual void Disconnect();
			virtual void Write(const std::wstring& msg);
			virtual std::wstring Read();
			virtual Raii::Win32Handle& GetInternalHandle();

			virtual std::wstring GetName() const;
			virtual DWORD GetSize() const;
			virtual bool IsOverlapped() const;
			virtual bool IsConnected() const;

		protected:
			virtual void Copy(const NamedPipe& other);
			virtual void Move(NamedPipe& other) noexcept;

		protected:
			Raii::Win32Handle m_pipe;
			std::wstring m_pipeName;
			DWORD m_size;
			bool m_isOverlapped;
			bool m_isConnected;
	};
}

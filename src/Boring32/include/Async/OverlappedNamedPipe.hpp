#pragma once
#include <string>
#include "../Raii/Raii.hpp"
#include "Event.hpp"
#include "OverlappedIo.hpp"

namespace Boring32::Async
{
	class OverlappedNamedPipe
	{
		public:
			virtual ~OverlappedNamedPipe();
			OverlappedNamedPipe(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances // PIPE_UNLIMITED_INSTANCES
			);

			OverlappedNamedPipe(const OverlappedNamedPipe& other);
			virtual void operator=(const OverlappedNamedPipe& other);

			OverlappedNamedPipe(OverlappedNamedPipe&& other) noexcept;
			virtual void operator=(OverlappedNamedPipe&& other) noexcept;

		public:
			virtual void Close();
			virtual OverlappedIo Connect();
			virtual void Disconnect();
			virtual OverlappedIo Write(const std::wstring& msg);
			virtual OverlappedIo Read(std::wstring& readData);
			virtual Raii::Win32Handle& GetInternalHandle();

			virtual std::wstring GetName() const;
			virtual DWORD GetSize() const;
			virtual DWORD GetMaxInstances() const;
			virtual bool IsConnected() const;

		protected:
			virtual void Copy(const OverlappedNamedPipe& other);
			virtual void Move(OverlappedNamedPipe& other) noexcept;

		protected:
			Raii::Win32Handle m_pipe;
			std::wstring m_pipeName;
			DWORD m_size;
			DWORD m_maxInstances;
			bool m_isConnected;
	};
}

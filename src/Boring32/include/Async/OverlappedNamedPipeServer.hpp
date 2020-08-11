#pragma once
#include <string>
#include "../Raii/Raii.hpp"
#include "Event.hpp"
#include "OverlappedIo.hpp"

namespace Boring32::Async
{
	class OverlappedNamedPipeServer
	{
		public:
			virtual ~OverlappedNamedPipeServer();
			OverlappedNamedPipeServer(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const bool isLocalPipe
			);
			OverlappedNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const DWORD openMode,
				const DWORD pipeMode
			);

			OverlappedNamedPipeServer(const OverlappedNamedPipeServer& other);
			virtual void operator=(const OverlappedNamedPipeServer& other);

			OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeServer&& other) noexcept;

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
			virtual DWORD GetPipeMode() const;
			virtual DWORD GetOpenMode() const;

		protected:
			virtual void InternalCreatePipe();
			virtual void Copy(const OverlappedNamedPipeServer& other);
			virtual void Move(OverlappedNamedPipeServer& other) noexcept;

		protected:
			Raii::Win32Handle m_pipe;
			std::wstring m_pipeName;
			DWORD m_size;
			DWORD m_maxInstances;
			bool m_isConnected;
			DWORD m_pipeMode;
			DWORD m_openMode;
	};
}

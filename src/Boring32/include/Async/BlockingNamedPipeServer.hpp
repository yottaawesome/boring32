#pragma once
#include <string>
#include "../Raii/Raii.hpp"

namespace Boring32::Async
{
	class BlockingNamedPipeServer
	{
		public:
			virtual ~BlockingNamedPipeServer();
			BlockingNamedPipeServer(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const bool isLocalPipe
			);
			BlockingNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const DWORD openMode,
				const DWORD pipeMode
			);

			BlockingNamedPipeServer(const BlockingNamedPipeServer& other);
			virtual void operator=(const BlockingNamedPipeServer& other);

			BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept;
			virtual void operator=(BlockingNamedPipeServer&& other) noexcept;

		public:
			virtual void Close();
			virtual void Connect();
			virtual void Disconnect();
			virtual void Write(const std::wstring& msg);
			virtual std::wstring Read();
			virtual Raii::Win32Handle& GetInternalHandle();

			virtual std::wstring GetName() const;
			virtual DWORD GetSize() const;
			virtual DWORD GetMaxInstances() const;
			virtual bool IsConnected() const;
			virtual DWORD GetPipeMode() const;
			virtual DWORD GetOpenMode() const;
			
		protected:
			virtual void InternalCreatePipe();
			virtual void Copy(const BlockingNamedPipeServer& other);
			virtual void Move(BlockingNamedPipeServer& other) noexcept;

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

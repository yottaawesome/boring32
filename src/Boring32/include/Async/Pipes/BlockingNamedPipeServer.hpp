#pragma once
#include <string>
#include "../../Raii/Raii.hpp"
#include "NamedPipeServerBase.hpp"

namespace Boring32::Async
{
	class BlockingNamedPipeServer : public NamedPipeServerBase
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
			virtual void Connect();
			virtual void Disconnect();
			virtual void Write(const std::wstring& msg);
			virtual std::wstring Read();
			
		protected:
			virtual void Copy(const BlockingNamedPipeServer& other);
			virtual void Move(BlockingNamedPipeServer& other) noexcept;
	};
}

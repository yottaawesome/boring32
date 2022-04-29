module;

#include <string>
#include <vector>
#include <Windows.h>

export module boring32.ipc:blockingnamedpipeserver;
import boring32.raii;
import :namedpipeserverbase;

export namespace Boring32::IPC
{
	class BlockingNamedPipeServer : public NamedPipeServerBase
	{
		public:
			virtual ~BlockingNamedPipeServer();
			BlockingNamedPipeServer(const BlockingNamedPipeServer& other);
			BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept;
			BlockingNamedPipeServer(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			);
			BlockingNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const DWORD openMode,
				const DWORD pipeMode
			);

		public:
			virtual void operator=(const BlockingNamedPipeServer& other);
			virtual void operator=(BlockingNamedPipeServer&& other) noexcept;

		public:
			virtual void Connect();
			virtual void Write(const std::wstring& msg);
			virtual bool Write(const std::wstring& msg, const std::nothrow_t&) noexcept;
			virtual std::wstring ReadAsString();
			virtual bool ReadAsString(std::wstring& out, const std::nothrow_t&) noexcept;

		protected:
			virtual void InternalWrite(const std::vector<std::byte>& msg);
			virtual std::vector<std::byte> InternalRead();
	};
}

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

			BlockingNamedPipeServer(const BlockingNamedPipeServer& other);
			virtual void operator=(const BlockingNamedPipeServer& other);

			BlockingNamedPipeServer(BlockingNamedPipeServer&& other) noexcept;
			virtual void operator=(BlockingNamedPipeServer&& other) noexcept;

		public:
			virtual void Connect();
			virtual void Write(const std::wstring& msg);
			virtual bool Write(const std::wstring& msg, const std::nothrow_t);
			virtual std::wstring Read();
			virtual bool Read(std::wstring& out, const std::nothrow_t);

		protected:
			virtual void InternalWrite(const std::wstring& msg);
			virtual std::wstring InternalRead();
	};
}

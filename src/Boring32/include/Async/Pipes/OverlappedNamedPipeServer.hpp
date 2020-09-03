#pragma once
#include <string>
#include "../../Raii/Raii.hpp"
#include "../Event.hpp"
#include "../OverlappedIo.hpp"
#include "NamedPipeServerBase.hpp"

namespace Boring32::Async
{
	class OverlappedNamedPipeServer : public NamedPipeServerBase
	{
		public:
			virtual ~OverlappedNamedPipeServer();
			OverlappedNamedPipeServer();
			OverlappedNamedPipeServer(
				const std::wstring& pipeName, 
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const bool isLocalPipe
			);
			OverlappedNamedPipeServer(
				const std::wstring& pipeName,
				const DWORD size,
				const DWORD maxInstances, // PIPE_UNLIMITED_INSTANCES
				const std::wstring& sid,
				const bool isInheritable,
				const DWORD openMode,
				const DWORD pipeMode
			);

			OverlappedNamedPipeServer(const OverlappedNamedPipeServer& other);
			virtual void operator=(const OverlappedNamedPipeServer& other);

			OverlappedNamedPipeServer(OverlappedNamedPipeServer&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeServer&& other) noexcept;

		public:
			virtual OverlappedOp Connect();
			virtual OverlappedIo Write(const std::wstring& msg);
			virtual OverlappedIo Write(const std::wstring& msg, const std::nothrow_t);
			virtual OverlappedIo Read(const DWORD noOfCharacters);
			virtual OverlappedIo Read(const DWORD noOfCharacters, const std::nothrow_t);

		protected:
			virtual OverlappedIo InternalWrite(const std::wstring& msg, const bool throwOnWin32Error);
			virtual OverlappedIo InternalRead(const DWORD noOfCharacters, const bool throwOnWin32Error);
	};
}

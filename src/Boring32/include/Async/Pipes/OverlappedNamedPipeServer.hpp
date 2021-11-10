#pragma once
#include <string>
#include "../Event.hpp"
#include "../OverlappedIo.hpp"
#include "NamedPipeServerBase.hpp"

import boring32.raii.win32handle;

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
			virtual void Connect(OverlappedOp& op);
			virtual bool Connect(OverlappedOp& op, std::nothrow_t) noexcept;
			virtual void Write(const std::wstring& msg, OverlappedIo& oio);
			virtual bool Write(const std::wstring& msg, OverlappedIo& op, std::nothrow_t) noexcept;
			virtual void Read(const DWORD noOfCharacters, OverlappedIo& oio);
			virtual bool Read(const DWORD noOfCharacters, OverlappedIo& oio, std::nothrow_t) noexcept;

		protected:
			virtual void InternalWrite(const std::wstring& msg, OverlappedIo& oio);
			virtual void InternalRead(const DWORD noOfCharacters, OverlappedIo& oio);
	};
}

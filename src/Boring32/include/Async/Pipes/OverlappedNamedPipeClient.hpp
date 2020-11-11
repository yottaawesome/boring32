#pragma once
#include "../OverlappedIo.hpp"
#include "NamedPipeClientBase.hpp"

namespace Boring32::Async
{
	class OverlappedNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~OverlappedNamedPipeClient();
			OverlappedNamedPipeClient();
			OverlappedNamedPipeClient(const std::wstring& name);

			OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other);
			virtual void operator=(const OverlappedNamedPipeClient& other);

			OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeClient&& other) noexcept;

		public:
			virtual void Write(const std::wstring& msg, OverlappedIo& op);
			virtual bool Write(const std::wstring& msg, OverlappedIo& op, std::nothrow_t) noexcept;
			virtual void Read(const DWORD noOfCharacters, OverlappedIo& op);
			virtual bool Read(const DWORD noOfCharacters, OverlappedIo& op, std::nothrow_t) noexcept;

		protected:
			virtual void InternalWrite(const std::wstring& msg, OverlappedIo& op);
			virtual void InternalRead(const DWORD noOfCharacters, OverlappedIo& op);
	};
}
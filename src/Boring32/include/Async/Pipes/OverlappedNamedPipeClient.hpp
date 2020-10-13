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
			virtual OverlappedIo Write(const std::wstring& msg);
			virtual bool Write(const std::wstring& msg, OverlappedIo& op) noexcept;
			virtual OverlappedIo Read(const DWORD noOfCharacters);
			virtual bool Read(const DWORD noOfCharacters, OverlappedIo& op) noexcept;

		protected:
			virtual OverlappedIo InternalWrite(const std::wstring& msg);
			virtual OverlappedIo InternalRead(const DWORD noOfCharacters);
	};
}
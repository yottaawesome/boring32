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

			virtual OverlappedIo Write(const std::wstring& msg);
			virtual OverlappedIo Write(const std::wstring& msg, const std::nothrow_t);
			virtual OverlappedIo Read(const DWORD noOfCharacters);
			virtual OverlappedIo Read(const DWORD noOfCharacters, const std::nothrow_t);

		protected:
			virtual OverlappedIo InternalWrite(const std::wstring& msg, const bool throwOnWin32Error);
			virtual OverlappedIo InternalRead(const DWORD noOfCharacters, const bool throwOnWin32Error);
	};
}
#pragma once
#include "../OverlappedIo.hpp"
#include "NamedPipeClientBase.hpp"

namespace Boring32::Async
{
	class OverlappedNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~OverlappedNamedPipeClient();
			OverlappedNamedPipeClient(const std::wstring& name);

			OverlappedNamedPipeClient(const OverlappedNamedPipeClient& other);
			virtual void operator=(const OverlappedNamedPipeClient& other);

			OverlappedNamedPipeClient(OverlappedNamedPipeClient&& other) noexcept;
			virtual void operator=(OverlappedNamedPipeClient&& other) noexcept;

			virtual OverlappedIo Write(const std::wstring& msg);
			virtual OverlappedIo Read(std::wstring& dataBuffer);
	};
}
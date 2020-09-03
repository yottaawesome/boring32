#pragma once
#include "NamedPipeClientBase.hpp"

namespace Boring32::Async
{
	class BlockingNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~BlockingNamedPipeClient();
			BlockingNamedPipeClient();
			BlockingNamedPipeClient(const std::wstring& name);

			BlockingNamedPipeClient(const BlockingNamedPipeClient& other);
			virtual void operator=(const BlockingNamedPipeClient& other);

			BlockingNamedPipeClient(BlockingNamedPipeClient&& other) noexcept;
			virtual void operator=(BlockingNamedPipeClient&& other) noexcept;

			virtual void Write(const std::wstring& msg);
			virtual bool Write(const std::wstring& msg, const std::nothrow_t);
			virtual std::wstring Read();
			virtual bool Read(std::wstring& out, const std::nothrow_t);

		protected:
			virtual void InternalWrite(const std::wstring& msg);
			virtual std::wstring InternalRead();
	};
}
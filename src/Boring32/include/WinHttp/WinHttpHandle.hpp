#pragma once
#include <memory>
#include <Windows.h>
#include <winhttp.h>

namespace Boring32::WinHttp
{
	class WinHttpHandle
	{
		public:
			virtual ~WinHttpHandle();
			WinHttpHandle();
			WinHttpHandle(HINTERNET handle);
			WinHttpHandle(WinHttpHandle&& other) noexcept;
			WinHttpHandle(const WinHttpHandle& other);

		public:
			virtual void operator=(WinHttpHandle&& other) noexcept;
			virtual void operator=(const HINTERNET handle);
			virtual void operator=(const WinHttpHandle&) = delete;
			virtual bool operator==(const HINTERNET other);

		public:
			virtual HINTERNET Get() const;
			virtual void Close();
			virtual void Copy(const WinHttpHandle& other);

		protected:
			std::shared_ptr<void> m_handle;
	};
}
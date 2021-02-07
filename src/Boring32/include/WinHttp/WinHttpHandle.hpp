#pragma once
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
			WinHttpHandle(const WinHttpHandle&) = delete;

		public:
			virtual void operator=(WinHttpHandle&& other) noexcept;
			virtual void operator=(const HINTERNET& copy);
			virtual void operator=(const WinHttpHandle&) = delete;
			
			virtual bool operator==(const HINTERNET other);

		public:
			virtual HINTERNET Get() const;
			virtual void Close();

		protected:
			HINTERNET m_handle;
	};
}
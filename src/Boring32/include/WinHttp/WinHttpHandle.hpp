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
			virtual void operator=(WinHttpHandle&& other) noexcept;

			WinHttpHandle(const WinHttpHandle&) = delete;
			virtual void operator=(const WinHttpHandle&) = delete;

			virtual void operator=(const HINTERNET& copy);
			virtual bool operator==(const HINTERNET other);
			virtual HINTERNET Get() const;
			virtual void Close();

		private:
			HINTERNET m_handle;
	};
}
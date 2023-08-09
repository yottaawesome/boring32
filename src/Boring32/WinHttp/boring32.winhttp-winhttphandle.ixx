export module boring32.winhttp:winhttphandle;
import std;

import <win32.hpp>;

export namespace Boring32::WinHttp
{
	std::shared_ptr<void> CreateCloseableWinHttpHandle(HINTERNET handle)
	{
		constexpr auto closeFunc = WinHttpCloseHandle;
		return { handle, closeFunc };
	}

	class WinHttpHandle
	{
		public:
			virtual ~WinHttpHandle() = default;
			WinHttpHandle() = default;
			WinHttpHandle(WinHttpHandle&& other) noexcept = default;
			WinHttpHandle(const WinHttpHandle& other) = default;
			WinHttpHandle(HINTERNET handle)
				: m_handle(CreateCloseableWinHttpHandle(handle))
			{ }
			
		public:
			virtual operator bool() const noexcept
			{
				return m_handle.get() != nullptr;
			}

			virtual bool operator==(const HINTERNET other) const noexcept
			{
				return m_handle.get() == other;
			}

			virtual WinHttpHandle& operator=(const HINTERNET handle)
			{
				m_handle = CreateCloseableWinHttpHandle(handle);
				return *this;
			}

			virtual WinHttpHandle& operator=(WinHttpHandle&& other) noexcept = default;
			virtual WinHttpHandle& operator=(const WinHttpHandle&) = default;
			
		public:
			virtual HINTERNET Get() const noexcept
			{
				return m_handle.get();
			}

			virtual void Close()
			{
				m_handle = nullptr;
			}

		protected:
			std::shared_ptr<void> m_handle;
	};
}
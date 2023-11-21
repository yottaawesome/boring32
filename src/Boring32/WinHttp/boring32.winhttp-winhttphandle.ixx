export module boring32.winhttp:winhttphandle;
import std;
import std.compat;
import boring32.win32;

export namespace Boring32::WinHttp
{
	std::shared_ptr<void> CreateCloseableWinHttpHandle(Win32::WinHttp::HINTERNET handle)
	{
		constexpr auto closeFunc = Win32::WinHttp::WinHttpCloseHandle;
		return { handle, closeFunc };
	}

	class WinHttpHandle final
	{
		public:
			~WinHttpHandle() = default;
			WinHttpHandle() = default;
			WinHttpHandle(WinHttpHandle&& other) noexcept = default;
			WinHttpHandle(const WinHttpHandle& other) = default;
			WinHttpHandle(Win32::WinHttp::HINTERNET handle)
				: m_handle(CreateCloseableWinHttpHandle(handle))
			{ }
			
		public:
			operator bool() const noexcept
			{
				return m_handle.get() != nullptr;
			}

			bool operator==(const Win32::WinHttp::HINTERNET other) const noexcept
			{
				return m_handle.get() == other;
			}

			WinHttpHandle& operator=(const Win32::WinHttp::HINTERNET handle)
			{
				m_handle = CreateCloseableWinHttpHandle(handle);
				return *this;
			}

			WinHttpHandle& operator=(WinHttpHandle&& other) noexcept = default;
			WinHttpHandle& operator=(const WinHttpHandle&) = default;
			
		public:
			Win32::WinHttp::HINTERNET Get() const noexcept
			{
				return m_handle.get();
			}

			void Close()
			{
				m_handle = nullptr;
			}

		private:
			std::shared_ptr<void> m_handle;
	};
}
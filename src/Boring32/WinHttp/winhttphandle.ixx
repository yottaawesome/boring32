export module boring32:winhttp.winhttphandle;
import std;
import :win32;

export namespace Boring32::WinHttp
{
	auto CreateCloseableWinHttpHandle(Win32::WinHttp::HINTERNET handle) -> std::shared_ptr<void>
	{
		constexpr auto closeFunc = Win32::WinHttp::WinHttpCloseHandle;
		return { handle, closeFunc };
	}

	class WinHttpHandle final
	{
	public:
		WinHttpHandle() = default;
		WinHttpHandle(const WinHttpHandle& other) = default;
		auto operator=(const WinHttpHandle&) -> WinHttpHandle& = default;
		WinHttpHandle(WinHttpHandle&& other) noexcept = default;
		auto operator=(WinHttpHandle&& other) noexcept -> WinHttpHandle& = default;

		WinHttpHandle(Win32::WinHttp::HINTERNET handle)
			: m_handle(CreateCloseableWinHttpHandle(handle))
		{ }
			
		operator bool() const noexcept
		{
			return m_handle.get() != nullptr;
		}

		auto operator==(Win32::WinHttp::HINTERNET other) const noexcept -> bool
		{
			return m_handle.get() == other;
		}

		auto operator=(const Win32::WinHttp::HINTERNET handle) -> WinHttpHandle&
		{
			m_handle = CreateCloseableWinHttpHandle(handle);
			return *this;
		}
			
		auto Get() const noexcept -> Win32::WinHttp::HINTERNET
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
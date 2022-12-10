module boring32.winhttp:winhttphandle;

namespace Boring32::WinHttp
{
	std::shared_ptr<void> CreateCloseableWinHttpHandle(HINTERNET handle)
	{
		return {
			handle,
			[](HINTERNET handle) { WinHttpCloseHandle(handle); }
		};
	}

	WinHttpHandle::WinHttpHandle(HINTERNET handle)
		: m_handle(CreateCloseableWinHttpHandle(handle))
	{ }

	WinHttpHandle& WinHttpHandle::operator=(const HINTERNET handle)
	{
		m_handle = CreateCloseableWinHttpHandle(handle);
		return *this;
	}

	void WinHttpHandle::Close()
	{
		m_handle = nullptr;
	}

	HINTERNET WinHttpHandle::Get() const noexcept
	{
		return m_handle.get();
	}

	bool WinHttpHandle::operator==(const HINTERNET other) const noexcept
	{
		return m_handle.get() == other;
	}

	WinHttpHandle::operator bool() const noexcept
	{
		return m_handle.get() != nullptr;
	}
}
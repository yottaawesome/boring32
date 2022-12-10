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

	WinHttpHandle::~WinHttpHandle()
	{
		Close();
	}

	WinHttpHandle::WinHttpHandle()
		: m_handle(nullptr)
	{ }

	WinHttpHandle::WinHttpHandle(HINTERNET handle)
		: m_handle(CreateCloseableWinHttpHandle(handle))
	{ }

	WinHttpHandle::WinHttpHandle(const WinHttpHandle& other)
	{
		Copy(other);
	}

	WinHttpHandle::WinHttpHandle(WinHttpHandle&& other) noexcept
	{
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	void WinHttpHandle::operator=(WinHttpHandle&& other) noexcept
	{
		Close();
		m_handle = other.m_handle;
		other.m_handle = nullptr;
	}

	void WinHttpHandle::operator=(const HINTERNET handle)
	{
		Close();
		m_handle = CreateCloseableWinHttpHandle(handle);
	}

	void WinHttpHandle::Close()
	{
		m_handle = nullptr;
	}

	HINTERNET WinHttpHandle::Get() const
	{
		return m_handle.get();
	}

	bool WinHttpHandle::operator==(const HINTERNET other)
	{
		return m_handle.get() == other;
	}

	void WinHttpHandle::Copy(const WinHttpHandle& other)
	{
		m_handle = other.m_handle;
	}
}
export module boring32:ipc.mailslotserver;
import std;
import :win32;
import :error;
import :raii;

export namespace Boring32::IPC
{
	class MailslotServer final
	{
	public:
		MailslotServer(std::wstring name, Win32::DWORD maxMessageSize, Win32::DWORD readTimeoutMs)
			: m_name(std::move(name)), m_maxMessageSize(maxMessageSize), m_readTimeoutMs(readTimeoutMs)
		{
			if (m_name.empty())
				throw Error::Boring32Error("Name cannot be empty.");

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createmailslotw
			m_handle = Win32::CreateMailslotW(m_name.c_str(), m_maxMessageSize, m_readTimeoutMs, nullptr);
			if (not m_handle)
				throw Error::Win32Error{Win32::GetLastError(), "CreateMailslotW() failed"};
		}

		MailslotServer(MailslotServer&&) noexcept = default;
		auto operator=(MailslotServer&&) noexcept -> MailslotServer& = default;

		void Close() noexcept
		{
			m_name.clear();
			m_maxMessageSize = 0;
			m_readTimeoutMs = 0;
			m_handle = nullptr;
		}
		constexpr auto GetHandle() const noexcept -> Win32::HANDLE 
		{ 
			return m_handle.GetHandle(); 
		}
		constexpr auto IsValid() const noexcept -> bool
		{
			return m_handle.GetHandle() != nullptr;
		}
		constexpr auto GetName() const noexcept -> std::wstring 
		{ 
			return m_name; 
		}
		constexpr auto GetMaxMessageSize() const noexcept -> Win32::DWORD 
		{
			return m_maxMessageSize;
		}
		constexpr auto GetReadTimeoutMs() const noexcept -> Win32::DWORD 
		{
			return m_readTimeoutMs;
		}

	private:
		std::wstring m_name;
		Win32::DWORD m_maxMessageSize;
		Win32::DWORD m_readTimeoutMs;
		RAII::UniqueHandle m_handle;
	};

	static_assert(not std::copyable<MailslotServer>);
	static_assert(std::movable<MailslotServer>);
}
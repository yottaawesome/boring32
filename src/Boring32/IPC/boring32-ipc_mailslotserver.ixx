export module boring32:ipc_mailslotserver;
import boring32.shared;
import :error;
import :raii;

export namespace Boring32::IPC
{
	struct MailslotServer final
	{
		MailslotServer(std::wstring name, unsigned long maxMessageSize, unsigned long readTimeoutMs, const std::wstring& dacl) 
			: m_name(std::move(name))
		{
			if (m_name.empty())
				throw Error::Boring32Error("Name cannot be empty.");

			// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createmailslotw
			m_handle = Win32::CreateMailslotW(m_name.c_str(), m_maxMessageSize, m_readTimeoutMs, nullptr);
			if (not m_handle)
				throw Error::Win32Error(Win32::GetLastError(), "CreateMailslotW() failed");
		}

		void Close() noexcept
		{
			m_name.clear();
			m_maxMessageSize = 0;
			m_readTimeoutMs = 0;
			m_handle = nullptr;
		}

		private:
		std::wstring m_name;
		unsigned long m_maxMessageSize;
		unsigned long m_readTimeoutMs;
		RAII::Win32Handle m_handle;
	};
}
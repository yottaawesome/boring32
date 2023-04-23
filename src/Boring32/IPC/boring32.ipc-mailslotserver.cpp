module boring32.ipc:mailslotserver;
import boring32.error;
import <win32.hpp>;

namespace Boring32::IPC
{
	MailslotServer::~MailslotServer()
	{
		Close();
	}

	MailslotServer::MailslotServer(
		std::wstring name,
		const unsigned long maxMessageSize,
		const unsigned long readTimeoutMs,
		const std::wstring& dacl
	)	: m_name(std::move(name))
	{
		if (m_name.empty())
			throw Error::Boring32Error("Name cannot be empty.");

		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createmailslotw
		m_handle = CreateMailslotW(
			m_name.c_str(),
			m_maxMessageSize,
			m_readTimeoutMs,
			nullptr
		);
		if (!m_handle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateMailslotW() failed", lastError);
		}
	}

	void MailslotServer::Close() noexcept
	{
		m_name.clear();
		m_maxMessageSize = 0;
		m_readTimeoutMs = 0;
		m_handle = nullptr;
	}
}
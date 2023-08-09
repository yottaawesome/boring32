export module boring32.ipc:mailslotserver;
import std;

import <win32.hpp>;
import boring32.error;
import boring32.raii;

export namespace Boring32::IPC
{
	class MailslotServer
	{
		public:
			virtual ~MailslotServer() = default;
			MailslotServer(const MailslotServer&) = default;
			MailslotServer(MailslotServer&&) noexcept = default;
			MailslotServer(
				std::wstring name,
				const unsigned long maxMessageSize,
				const unsigned long readTimeoutMs,
				const std::wstring& dacl
			) : m_name(std::move(name))
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

		public:
			virtual MailslotServer& operator=(const MailslotServer&) = default;
			virtual MailslotServer& operator=(MailslotServer&&) noexcept = default;

		public:
			virtual void Close() noexcept
			{
				m_name.clear();
				m_maxMessageSize = 0;
				m_readTimeoutMs = 0;
				m_handle = nullptr;
			}

		protected:
			std::wstring m_name;
			unsigned long m_maxMessageSize;
			unsigned long m_readTimeoutMs;
			RAII::Win32Handle m_handle;
	};
}
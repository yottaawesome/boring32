export module boring32.ipc:mailslotserver;
import boring32.raii;
import <string>;

export namespace Boring32::IPC
{
	class MailslotServer
	{
		public:
			virtual ~MailslotServer();
			MailslotServer(const MailslotServer&) = default;
			MailslotServer(MailslotServer&&) noexcept = default;
			MailslotServer(
				std::wstring name,
				const unsigned long maxMessageSize,
				const unsigned long readTimeoutMs,
				const std::wstring& dacl
			);

		public:
			virtual MailslotServer& operator=(const MailslotServer&) = default;
			virtual MailslotServer& operator=(MailslotServer&&) noexcept = default;

		public:
			virtual void Close() noexcept;

		protected:
			std::wstring m_name;
			unsigned long m_maxMessageSize;
			unsigned long m_readTimeoutMs;
			RAII::Win32Handle m_handle;
	};
}
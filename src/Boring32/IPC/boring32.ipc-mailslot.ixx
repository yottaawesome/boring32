module;

#include <string>

export module boring32.ipc:mailslot;
import boring32.raii;

export namespace Boring32::IPC
{
	class Mailslot
	{
		public:
			virtual ~Mailslot();
			Mailslot(
				std::wstring name,
				const unsigned long maxMessageSize,
				const unsigned long readTimeoutMs,
				const std::wstring& dacl
			);

		public:
			virtual void Close();

		protected:
			std::wstring m_name;
			unsigned long m_maxMessageSize;
			unsigned long m_readTimeoutMs;
			Raii::Win32Handle m_handle;
	};
}
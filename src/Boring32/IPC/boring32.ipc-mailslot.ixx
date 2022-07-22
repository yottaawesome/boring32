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
			Mailslot(const Mailslot&) = default;
			Mailslot(Mailslot&&) noexcept = default;
			Mailslot(
				std::wstring name,
				const unsigned long maxMessageSize,
				const unsigned long readTimeoutMs,
				const std::wstring& dacl
			);

		public:
			virtual Mailslot& operator=(const Mailslot&) = default;
			virtual Mailslot& operator=(Mailslot&&) noexcept = default;

		public:
			virtual void Close();

		protected:
			std::wstring m_name;
			unsigned long m_maxMessageSize;
			unsigned long m_readTimeoutMs;
			Raii::Win32Handle m_handle;
	};
}
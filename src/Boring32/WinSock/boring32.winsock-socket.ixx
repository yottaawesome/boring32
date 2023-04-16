export module boring32.winsock:socket;
import "win32.hpp";

export namespace Boring32::WinSock
{
	class Socket final
	{
		public:
			~Socket();
			Socket() = default;
			Socket(const Socket&) = delete;
			Socket(Socket&&) noexcept;
			Socket(const SOCKET socket);
			void Close();

		private:
			SOCKET m_socket = INVALID_SOCKET;
	};
}

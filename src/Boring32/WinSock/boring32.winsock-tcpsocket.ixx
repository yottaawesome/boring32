export module boring32.winsock:tcpsocket;
import :uniqueptrs;
import <string>;
import <vector>;
import <win32.hpp>;

export namespace Boring32::WinSock
{
	// Based on https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-client-application
	class TCPSocket
	{
		public:
			static const SOCKET InvalidSocket;

		public:
			virtual ~TCPSocket();
			TCPSocket();
			TCPSocket(const TCPSocket& other) = delete;
			TCPSocket(TCPSocket&& other) noexcept;
			TCPSocket(const std::wstring host, const unsigned portNumber);

		public:
			virtual TCPSocket& operator=(const TCPSocket& other) = delete;
			virtual TCPSocket& operator=(TCPSocket&& other) noexcept;

		public:
			virtual void Open();
			virtual void Connect();
			virtual void Connect(const DWORD socketTTL, const DWORD maxRetryTimeout);
			virtual void Close();
			virtual void Send(const std::vector<std::byte>& data);
			virtual std::vector<std::byte> Receive(const unsigned bytesToRead);
			virtual void SetSocketTTL(const DWORD ttl);
			virtual void SetMaxRetryTimeout(const DWORD timeoutSeconds);

		public:
			virtual const std::wstring& GetHost() const noexcept;
			virtual unsigned GetPort() const noexcept;
			virtual SOCKET GetHandle() const noexcept;

		protected:
			virtual TCPSocket& Move(TCPSocket& other) noexcept;

		protected:
			std::wstring m_host;
			unsigned m_portNumber;
			SOCKET m_socket; // doesn't work with unique_ptr
			int m_addressFamily;
	};		AddrInfoWUniquePtr m_addrPtr;

}
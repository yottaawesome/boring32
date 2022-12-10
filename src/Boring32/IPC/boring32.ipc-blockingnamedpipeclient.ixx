export module boring32.ipc:blockingnamedpipeclient;
import :namedpipeclientbase;
import <vector>;
import <string>;

export namespace Boring32::IPC
{
	class BlockingNamedPipeClient : public NamedPipeClientBase
	{
		public:
			virtual ~BlockingNamedPipeClient();
			BlockingNamedPipeClient();
			BlockingNamedPipeClient(const BlockingNamedPipeClient& other);
			BlockingNamedPipeClient(BlockingNamedPipeClient&& other) noexcept;
			BlockingNamedPipeClient(const std::wstring& name);

		public:
			virtual void operator=(const BlockingNamedPipeClient& other);
			virtual void operator=(BlockingNamedPipeClient&& other) noexcept;

		public:
			virtual void Write(const std::wstring& msg);
			virtual bool Write(const std::wstring& msg, const std::nothrow_t) noexcept;
			virtual void Write(const std::vector<std::byte>& data);
			virtual bool Write(const std::vector<std::byte>& data, const std::nothrow_t)  noexcept;

			virtual std::wstring ReadAsString();
			virtual bool ReadAsString(std::wstring& out, const std::nothrow_t) noexcept;

		protected:
			virtual void InternalWrite(const std::vector<std::byte>& data);
			virtual std::vector<std::byte> InternalRead();
	};
}
export module boring32.services:service;
import :raii;
import <vector>;
import <string>;
import <Windows.h>;

export namespace Boring32::Services
{
	class Service
	{
		public:
			virtual ~Service();
			Service() = default;
			Service(const Service&) = default;
			Service(Service&&) noexcept = default;
			Service(ServiceHandleSharedPtr service);

		public:
			virtual Service& operator=(const Service&) = default;
			virtual Service& operator=(Service&&) noexcept = default;

		public:
			virtual void Start(const std::vector<std::wstring>& args);
			virtual void Stop();
			virtual void Delete();
			virtual std::wstring GetDisplayName() const;
			virtual bool IsRunning() const;
			virtual SC_HANDLE GetHandle() const noexcept;
			virtual void SendControlCode(
				const unsigned long controlCode,
				const std::wstring& comment
			);

		protected:
			virtual std::vector<std::byte> GetConfigBuffer() const;

		protected:
			ServiceHandleSharedPtr m_service;
	};
}
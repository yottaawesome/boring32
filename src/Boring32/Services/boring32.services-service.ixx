module;

#include <vector>
#include <string>

export module boring32.services:service;
import :raii;

export namespace Boring32::Services
{
	class Service
	{
		public:
			virtual ~Service();
			Service(const Service&) = default;
			Service(Service&&) noexcept = default;
			Service(ServiceHandleSharedPtr service);

		public:
			virtual Service& operator=(const Service&) = default;
			virtual Service& operator=(Service&&) noexcept = default;

		public:
			virtual void Start(std::vector<std::wstring>& args); 
			virtual void Stop();
			virtual void Delete();
			virtual std::wstring GetDisplayName() const;
			virtual bool IsRunning() const;

		protected:
			virtual std::vector<std::byte> GetConfigBuffer() const;

		protected:
			ServiceHandleSharedPtr m_service;
	};
}
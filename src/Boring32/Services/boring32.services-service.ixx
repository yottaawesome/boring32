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
			Service(ServiceHandleSharedPtr service);

		public:
			virtual void Start(std::vector<std::wstring>& args); 
			virtual void Stop();
			virtual void Delete();
			virtual std::wstring GetDisplayName() const;

		protected:
			virtual std::vector<std::byte> GetConfigBuffer() const;

		protected:
			ServiceHandleSharedPtr m_service;
	};
}
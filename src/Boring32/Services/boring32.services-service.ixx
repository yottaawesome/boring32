export module boring32.services:service;
import :raii;

export namespace Boring32::Services
{
	class Service
	{
		public:
			Service(ServiceHandleSharedPtr service);

		protected:
			ServiceHandleSharedPtr m_service;
	};
}
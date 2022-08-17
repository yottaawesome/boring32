export module boring32.services:servicecontrolmanager;
import :raii;

export namespace Boring32::Services
{
	class ServiceControlManager
	{
		public:
			virtual ~ServiceControlManager();
			ServiceControlManager(const ServiceControlManager&) = default;
			ServiceControlManager(ServiceControlManager&&) noexcept = default;
			ServiceControlManager();
			ServiceControlManager(const unsigned desiredAccess);

		public:
			virtual ServiceControlManager& operator=(const ServiceControlManager&) = default;
			virtual ServiceControlManager& operator=(ServiceControlManager&&) noexcept = default;

		public:
			virtual void Close();

		protected:
			virtual void Open(const unsigned desiredAccess);

		protected:
			ServiceHandleSharedPtr m_scm;
	};
}

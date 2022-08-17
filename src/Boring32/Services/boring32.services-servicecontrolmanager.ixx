export module boring32.services:servicecontrolmanager;
import :raii;

export namespace Boring32::Services
{
	class ServiceControlManager
	{
		public:
			virtual ~ServiceControlManager();
			ServiceControlManager();
			ServiceControlManager(const unsigned desiredAccess);

		protected:
			virtual void Open(const unsigned desiredAccess);
			virtual void Close();

		protected:
			ServiceHandleSharedPtr m_scm;
	};
}

export module boring32.services:servicecontrolmanager;
import :raii;

export namespace Boring32::Services
{
	class ServiceControlManager
	{
		public:
			virtual ~ServiceControlManager();
			ServiceControlManager();

		protected:
			virtual void Open();
			virtual void Close();

		protected:
			ServiceHandleSharedPtr m_scm;
	};
}

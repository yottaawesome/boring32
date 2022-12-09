export module boring32.services:servicecontrolmanager;
import :raii;
import :service;
import <string>;
import <Windows.h>;

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
			virtual operator bool() const noexcept;

		public:
			virtual void Close();
			virtual Service AccessService(
				const std::wstring& name
			);
			virtual Service AccessService(
				const std::wstring& name,
				const unsigned desiredAccess
			);
			virtual SC_HANDLE GetHandle() const noexcept;

		protected:
			virtual void Open(const unsigned desiredAccess);

		protected:
			ServiceHandleSharedPtr m_scm;
	};
}

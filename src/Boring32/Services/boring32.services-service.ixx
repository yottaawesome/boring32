export module boring32.services:service;
import :raii;
import <vector>;
import <string>;
import <win32.hpp>;
import boring32.error;
import :raii;
//import <source_location>;
import <utility>;

export namespace Boring32::Services
{
	class Service
	{
		public:
			virtual ~Service() = default;
			Service() = default;
			Service(const Service&) = default;
			Service(Service&&) noexcept = default;
			Service(ServiceHandleSharedPtr service)
				: m_service(std::move(service))
			{
				if (!m_service)
					throw Error::Boring32Error("service parameter cannot be null");
			}

		public:
			virtual Service& operator=(const Service&) = default;
			virtual Service& operator=(Service&&) noexcept = default;

		public:
			virtual void Start(const std::vector<std::wstring>& args)
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				std::vector<LPCWSTR> argv;
				for (const std::wstring& arg : args)
				{
					if (arg.empty())
						continue;
					argv.push_back(&arg[0]);
				}

				const bool succeeded = StartServiceW(
					m_service.get(),
					static_cast<DWORD>(argv.size()),
					&argv[0]
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("StartServiceW() failed", lastError);
				}
			}

			virtual void Stop()
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
				SERVICE_CONTROL_STATUS_REASON_PARAMS params
				{
					.dwReason = SERVICE_STOP_REASON_FLAG_PLANNED & SERVICE_STOP_REASON_MAJOR_NONE & SERVICE_STOP_REASON_MINOR_NONE,
				};
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
				const bool succeeded = ControlServiceExW(
					m_service.get(),
					SERVICE_CONTROL_STOP,
					SERVICE_CONTROL_STATUS_REASON_INFO,
					&params
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ControlServiceExW() failed", lastError);
				}
			}

			virtual void Delete()
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				if (!DeleteService(m_service.get()))
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("service parameter cannot be null", lastError);
				}
				m_service = nullptr;
			}

			virtual std::wstring GetDisplayName() const
			{
				std::vector<std::byte> buffer = GetConfigBuffer();
				auto config = reinterpret_cast<QUERY_SERVICE_CONFIGW*>(&buffer[0]);
				return config->lpDisplayName;
			}

			virtual bool IsRunning() const
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
				SERVICE_STATUS_PROCESS status{ 0 };
				DWORD bufSize;
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
				const bool succeeded = QueryServiceStatusEx(
					m_service.get(),
					// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-scmr/a7de3a4b-0b9e-4b9b-8863-b3dbc9bbe02b
					SC_STATUS_TYPE::SC_STATUS_PROCESS_INFO,
					reinterpret_cast<LPBYTE>(&status),
					sizeof(status),
					&bufSize
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("QueryServiceStatusEx() failed", lastError);
				}
				return status.dwCurrentState == SERVICE_RUNNING;
			}
			
			virtual SC_HANDLE GetHandle() const noexcept
			{
				return m_service.get();
			}

			virtual void SendControlCode(
				const unsigned long controlCode,
				const std::wstring& comment
			)
			{

				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
				SERVICE_CONTROL_STATUS_REASON_PARAMS params
				{
					.pszComment = const_cast<wchar_t*>(comment.c_str())
				};
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
				const bool succeeded = ControlServiceExW(
					m_service.get(),
					controlCode,
					SERVICE_CONTROL_STATUS_REASON_INFO,
					&params
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("ControlServiceExW() failed", lastError);
				}
			}

		protected:
			virtual std::vector<std::byte> GetConfigBuffer() const
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				DWORD bytesNeeded = 0;
				bool succeeded = QueryServiceConfigW(
					m_service.get(),
					nullptr,
					0,
					&bytesNeeded
				);
				const auto lastError = GetLastError();
				if (lastError != ERROR_INSUFFICIENT_BUFFER)
					throw Error::Win32Error("QueryServiceConfigW() failed", lastError);

				std::vector<std::byte> buffer(bytesNeeded);
				succeeded = QueryServiceConfigW(
					m_service.get(),
					reinterpret_cast<QUERY_SERVICE_CONFIGW*>(&buffer[0]),
					static_cast<DWORD>(buffer.size()),
					&bytesNeeded
				);
				if (!succeeded)
				{
					const auto lastError = GetLastError();
					throw Error::Win32Error("QueryServiceConfigW() failed", lastError);
				}
				return buffer;
			}

		protected:
			ServiceHandleSharedPtr m_service;
	};
}
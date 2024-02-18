export module boring32.services:service;
import boring32.shared;
import boring32.error;
import :raii;

export namespace Boring32::Services
{
	class Service final
	{
		public:
			~Service() = default;
			Service() = default;
			Service(const Service&) = default;
			Service& operator=(const Service&) = default;
			Service(Service&&) noexcept = default;
			Service& operator=(Service&&) noexcept = default;

		public:
			Service(ServiceHandleSharedPtr service)
				: m_service(std::move(service))
			{
				if (!m_service)
					throw Error::Boring32Error("service parameter cannot be null");
			}

		public:
			void Start(const std::vector<std::wstring>& args)
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				std::vector<Win32::LPCWSTR> argv;
				for (const std::wstring& arg : args)
				{
					if (arg.empty())
						continue;
					argv.push_back(&arg[0]);
				}

				const bool succeeded = Win32::StartServiceW(
					m_service.get(),
					static_cast<Win32::DWORD>(argv.size()),
					&argv[0]
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("StartServiceW() failed", lastError);
				}
			}

			void Stop()
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
				Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS params
				{
					.dwReason = Win32::_SERVICE_STOP_REASON_FLAG_PLANNED & Win32::_SERVICE_STOP_REASON_MAJOR_NONE & Win32::_SERVICE_STOP_REASON_MINOR_NONE,
				};
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
				const bool succeeded = Win32::ControlServiceExW(
					m_service.get(),
					Win32::_SERVICE_CONTROL_STOP,
					Win32::_SERVICE_CONTROL_STATUS_REASON_INFO,
					&params
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("ControlServiceExW() failed", lastError);
				}
			}

			void Delete()
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				if (!Win32::DeleteService(m_service.get()))
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("service parameter cannot be null", lastError);
				}
				m_service = nullptr;
			}

			std::wstring GetDisplayName() const
			{
				std::vector<std::byte> buffer = GetConfigBuffer();
				auto config = reinterpret_cast<Win32::QUERY_SERVICE_CONFIGW*>(&buffer[0]);
				return config->lpDisplayName;
			}

			bool IsRunning() const
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
				Win32::SERVICE_STATUS_PROCESS status{ 0 };
				Win32::DWORD bufSize;
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
				const bool succeeded = Win32::QueryServiceStatusEx(
					m_service.get(),
					// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-scmr/a7de3a4b-0b9e-4b9b-8863-b3dbc9bbe02b
					Win32::SC_STATUS_TYPE::SC_STATUS_PROCESS_INFO,
					reinterpret_cast<LPBYTE>(&status),
					sizeof(status),
					&bufSize
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("QueryServiceStatusEx() failed", lastError);
				}
				return status.dwCurrentState == Win32::_SERVICE_RUNNING;
			}
			
			Win32::SC_HANDLE GetHandle() const noexcept
			{
				return m_service.get();
			}

			void SendControlCode(
				const unsigned long controlCode,
				const std::wstring& comment
			)
			{

				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
				Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS params
				{
					.pszComment = const_cast<wchar_t*>(comment.c_str())
				};
				// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
				const bool succeeded = Win32::ControlServiceExW(
					m_service.get(),
					controlCode,
					Win32::_SERVICE_CONTROL_STATUS_REASON_INFO,
					&params
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("ControlServiceExW() failed", lastError);
				}
			}

		private:
			std::vector<std::byte> GetConfigBuffer() const
			{
				if (!m_service)
					throw Error::Boring32Error("m_service is nullptr");

				Win32::DWORD bytesNeeded = 0;
				bool succeeded = Win32::QueryServiceConfigW(
					m_service.get(),
					nullptr,
					0,
					&bytesNeeded
				);
				const auto lastError = Win32::GetLastError();
				if (lastError != Win32::ErrorCodes::InsufficientBuffer)
					throw Error::Win32Error("QueryServiceConfigW() failed", lastError);

				std::vector<std::byte> buffer(bytesNeeded);
				succeeded = QueryServiceConfigW(
					m_service.get(),
					reinterpret_cast<Win32::QUERY_SERVICE_CONFIGW*>(&buffer[0]),
					static_cast<Win32::DWORD>(buffer.size()),
					&bytesNeeded
				);
				if (!succeeded)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("QueryServiceConfigW() failed", lastError);
				}
				return buffer;
			}

		private:
			ServiceHandleSharedPtr m_service;
	};
}
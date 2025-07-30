export module boring32:services_service;
import std;
import boring32.win32;
import :error;
import :services_raii;

export namespace Boring32::Services
{
	struct Service final
	{
		Service() = default;
		Service(const Service&) = default;
		Service& operator=(const Service&) = default;
		Service(Service&&) noexcept = default;
		Service& operator=(Service&&) noexcept = default;

		Service(ServiceHandleSharedPtr service)
			: m_service(std::move(service))
		{
			if (not m_service)
				throw Error::Boring32Error("service parameter cannot be null");
		}

		void Start(const std::vector<std::wstring>& args)
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			std::vector<Win32::LPCWSTR> argv;
			for (const std::wstring& arg : args)
			{
				if (arg.empty())
					continue;
				argv.push_back(&arg[0]);
			}

			bool succeeded = Win32::StartServiceW(
				m_service.get(),
				static_cast<Win32::DWORD>(argv.size()),
				&argv[0]
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "StartServiceW() failed");
		}

		void Stop()
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw

			constexpr auto reason =
				Win32::Services::StopReason::FlagPlanned
				& Win32::Services::StopReason::MajorNone
				& Win32::Services::StopReason::MinorNone;
			Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS params{ .dwReason = reason };
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
			bool succeeded = Win32::ControlServiceExW(
				m_service.get(),
				Win32::Services::Control::Stop,
				Win32::Services::Control::StatusReasonInfo,
				&params
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(),"ControlServiceExW() failed");
		}

		void Delete()
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");
			if (not Win32::DeleteService(m_service.get()))
				throw Error::Win32Error(Win32::GetLastError(), "service parameter cannot be null");
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
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
			Win32::SERVICE_STATUS_PROCESS status{ 0 };
			Win32::DWORD bufSize;
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
			bool succeeded = Win32::QueryServiceStatusEx(
				m_service.get(),
				// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-scmr/a7de3a4b-0b9e-4b9b-8863-b3dbc9bbe02b
				Win32::SC_STATUS_TYPE::SC_STATUS_PROCESS_INFO,
				reinterpret_cast<LPBYTE>(&status),
				sizeof(status),
				&bufSize
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "QueryServiceStatusEx() failed");
			return status.dwCurrentState == Win32::_SERVICE_RUNNING;
		}
			
		Win32::SC_HANDLE GetHandle() const noexcept
		{
			return m_service.get();
		}

		void SendControlCode(unsigned long controlCode, const std::wstring& comment)
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
			Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS params {
				.pszComment = const_cast<wchar_t*>(comment.c_str())
			};
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
			bool succeeded = Win32::ControlServiceExW(
				m_service.get(),
				controlCode,
				Win32::Services::Control::StatusReasonInfo,
				&params
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "ControlServiceExW() failed");
		}

	private:
		std::vector<std::byte> GetConfigBuffer() const
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			Win32::DWORD bytesNeeded = 0;
			bool succeeded = Win32::QueryServiceConfigW(m_service.get(), nullptr, 0, &bytesNeeded);
			if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error(lastError, "QueryServiceConfigW() failed");

			std::vector<std::byte> buffer(bytesNeeded);
			succeeded = Win32::QueryServiceConfigW(
				m_service.get(),
				reinterpret_cast<Win32::QUERY_SERVICE_CONFIGW*>(&buffer[0]),
				static_cast<Win32::DWORD>(buffer.size()),
				&bytesNeeded
			);
			if (not succeeded)
				throw Error::Win32Error(Win32::GetLastError(), "QueryServiceConfigW() failed");
			return buffer;
		}

		ServiceHandleSharedPtr m_service;
	};
}
export module boring32:services.service;
import std;
import :win32;
import :error;
import :services.raii;

export namespace Boring32::Services
{
	class Service final
	{
	public:
		Service() = default;
		Service(const Service&) = delete;
		Service& operator=(const Service&) = delete;
		Service(Service&&) noexcept = default;
		Service& operator=(Service&&) noexcept = default;

		Service(ServiceHandleUniquePtr service)
			: m_service(std::move(service))
		{
			if (not m_service)
				throw Error::Boring32Error{"service parameter cannot be null"};
		}

		void Start(const std::vector<std::wstring>& args)
		{
			if (not m_service)
				throw Error::Boring32Error{"m_service is nullptr"};

			auto argv = args 
				| std::ranges::views::filter([](auto&& wstring) { return not wstring.empty(); }) 
				| std::ranges::views::transform([](auto&& wstring) -> Win32::LPCWSTR { return wstring.data(); })
				| std::ranges::to<std::vector<Win32::LPCWSTR>>();
			auto succeeded = Win32::StartServiceW(
				m_service.get(),
				static_cast<Win32::DWORD>(argv.size()),
				&argv[0]
			);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "StartServiceW() failed"};
		}

		void Stop()
		{
			if (not m_service)
				throw Error::Boring32Error{"m_service is nullptr"};

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
			constexpr auto reason =
				Win32::Services::StopReason::FlagPlanned
				| Win32::Services::StopReason::MajorNone
				| Win32::Services::StopReason::MinorNone;
			auto params = Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS{ .dwReason = reason };
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
			auto succeeded = 
				Win32::ControlServiceExW(
					m_service.get(),
					Win32::Services::Control::Stop,
					Win32::Services::Control::StatusReasonInfo,
					&params
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(),"ControlServiceExW() failed"};
		}

		void Delete()
		{
			if (not m_service)
				throw Error::Boring32Error{"m_service is nullptr"};
			if (not Win32::DeleteService(m_service.get()))
				throw Error::Win32Error{Win32::GetLastError(), "DeleteService() failed."};
			m_service = nullptr;
		}

		[[nodiscard]]
		auto GetDisplayName() const -> std::wstring
		{
			auto buffer = std::vector{ GetConfigBuffer() };
			auto config = reinterpret_cast<Win32::QUERY_SERVICE_CONFIGW*>(&buffer[0]);
			return config->lpDisplayName;
		}

		[[nodiscard]]
		auto IsRunning() const -> bool
		{
			if (not m_service)
				throw Error::Boring32Error("m_service is nullptr");

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_status_process
			auto status = Win32::SERVICE_STATUS_PROCESS{};
			auto bufSize = Win32::DWORD{};
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-queryservicestatusex
			auto succeeded = 
				Win32::QueryServiceStatusEx(
					m_service.get(),
					// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-scmr/a7de3a4b-0b9e-4b9b-8863-b3dbc9bbe02b
					Win32::SC_STATUS_TYPE::SC_STATUS_PROCESS_INFO,
					reinterpret_cast<LPBYTE>(&status),
					sizeof(status),
					&bufSize
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "QueryServiceStatusEx() failed"};
			return status.dwCurrentState == Win32::ServiceRunning;
		}
			
		[[nodiscard]]
		auto GetHandle() const noexcept -> Win32::SC_HANDLE
		{
			return m_service.get();
		}

		void SendControlCode(unsigned long controlCode, const std::wstring& comment)
		{
			if (not m_service)
				throw Error::Boring32Error{"m_service is nullptr"};

			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/ns-winsvc-service_control_status_reason_paramsw
			auto params = Win32::SERVICE_CONTROL_STATUS_REASON_PARAMS{
				.pszComment = const_cast<wchar_t*>(comment.c_str())
			};
			// https://learn.microsoft.com/en-us/windows/win32/api/winsvc/nf-winsvc-controlserviceexw
			auto succeeded = 
				Win32::ControlServiceExW(
					m_service.get(),
					controlCode,
					Win32::Services::Control::StatusReasonInfo,
					&params
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "ControlServiceExW() failed"};
		}

	private:
		[[nodiscard]]
		auto GetConfigBuffer() const -> std::vector<std::byte>
		{
			if (not m_service)
				throw Error::Boring32Error{"m_service is nullptr"};

			auto bytesNeeded = Win32::DWORD{};
			auto succeeded = Win32::QueryServiceConfigW(m_service.get(), nullptr, 0, &bytesNeeded);
			if (auto lastError = Win32::GetLastError(); lastError != Win32::ErrorCodes::InsufficientBuffer)
				throw Error::Win32Error{lastError, "QueryServiceConfigW() failed"};

			auto buffer = std::vector<std::byte>(bytesNeeded);
			succeeded = 
				Win32::QueryServiceConfigW(
					m_service.get(),
					reinterpret_cast<Win32::QUERY_SERVICE_CONFIGW*>(&buffer[0]),
					static_cast<Win32::DWORD>(buffer.size()),
					&bytesNeeded
				);
			if (not succeeded)
				throw Error::Win32Error{Win32::GetLastError(), "QueryServiceConfigW() failed"};
			return buffer;
		}

		ServiceHandleUniquePtr m_service;
	};
}
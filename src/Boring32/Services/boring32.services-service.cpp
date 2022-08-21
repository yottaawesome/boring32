module;

#include <source_location>
#include <string>
#include <utility>
#include <Windows.h>

module boring32.services:service;
import boring32.error;
import :raii;

namespace Boring32::Services
{
	Service::~Service() {}

	Service::Service(ServiceHandleSharedPtr service)
		: m_service(std::move(service))
	{
		if (!m_service)
			throw Error::Boring32Error("service parameter cannot be null");
	}

	void Service::Start(std::vector<std::wstring>& args)
	{
		std::vector<LPCWSTR> argv;
		for (std::wstring& arg : args)
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
			throw Error::Win32Error("StartServiceW()", lastError);
		}
	}

	void Service::Stop()
	{
	}

	void Service::Delete()
	{
		if (!DeleteService(m_service.get()))
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("service parameter cannot be null", lastError);
		}
	}

	std::wstring Service::GetDisplayName() const
	{
		std::vector<std::byte> buffer = GetConfigBuffer();
		auto config = reinterpret_cast<QUERY_SERVICE_CONFIGW*>(&buffer[0]);
		return config->lpDisplayName;
	}

	std::vector<std::byte> Service::GetConfigBuffer() const
	{
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
}
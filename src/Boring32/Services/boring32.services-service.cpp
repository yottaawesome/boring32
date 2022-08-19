module;

#include <source_location>
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
			throw Boring32:Error("service parameter cannot be null");
	}

	void Service::Start()
	{

	}

	void Service::Stop()
	{

	}

	void Service::Delete()
	{
		if (!DeleteService(m_service.get())
		{
			const auto lastError = GetLastError();
			throw Boring32:Error("service parameter cannot be null", lastError);
		}
	}
}
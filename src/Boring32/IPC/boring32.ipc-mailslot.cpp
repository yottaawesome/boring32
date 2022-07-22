module;

#include <string>
#include <source_location>
#include <Windows.h>

module boring32.ipc:mailslot;
import boring32.error;

namespace Boring32::IPC
{
	Mailslot::~Mailslot()
	{
		Close();
	}

	Mailslot::Mailslot(
		std::wstring name,
		const unsigned long maxMessageSize,
		const unsigned long readTimeoutMs,
		const std::wstring& dacl
	)	: m_name(std::move(name))
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createmailslotw
		m_handle = CreateMailslotW(
			m_name.c_str(),
			m_maxMessageSize,
			m_readTimeoutMs,
			nullptr
		);
		if (!m_handle)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("CreateMailslotW() failed", lastError);
		}
	}

	void Mailslot::Close()
	{
		m_name.clear();
		m_maxMessageSize = 0;
		m_readTimeoutMs = 0;
		m_handle = nullptr;
	}
}
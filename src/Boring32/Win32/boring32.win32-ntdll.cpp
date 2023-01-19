module;

#include <source_location>;

module boring32.win32:ntdll;
import boring32.error;
import <win32.hpp>;

namespace Boring32::Win32
{
	NTDLL::NTDLL() 
	{
		Map(); 
	}

	void NTDLL::Map()
	{
		HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
		if (!ntdll)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetModuleHandleW() failed", lastError);
		}

		m_mapViewOfSection = (MapViewOfSection)GetProcAddress(ntdll, "NtMapViewOfSection");
		if (!m_mapViewOfSection)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcAddress() failed", lastError);
		}

		m_querySystemInformation = (QuerySystemInformation)GetProcAddress(ntdll, "NtQuerySystemInformation");
		if (!m_querySystemInformation)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcAddress() failed", lastError);
		}
		
		m_duplicateObject = (DuplicateObject)GetProcAddress(ntdll, "NtDuplicateObject");
		if (!m_duplicateObject)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcAddress() failed", lastError);
		}

		m_queryObject = (QueryObject)GetProcAddress(ntdll, "NtQueryObject");
		if (!m_queryObject)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("GetProcAddress() failed", lastError);
		}
	}
}

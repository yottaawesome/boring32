export module boring32:native.ntdll;
import :win32;
import :error;
import :native.defs;

export namespace Boring32::Native
{
	struct NTDLL final
	{
		NTDLL()
		{
			Map();
		}

		NTDLL(const NTDLL&) = delete;
		NTDLL(NTDLL&&) noexcept = delete;

		NTDLL& operator=(const NTDLL&) = delete;
		NTDLL& operator=(NTDLL&&) noexcept = delete;

		private:
		void Map()
		{
			Win32::HMODULE ntdll = Win32::GetModuleHandleW(L"ntdll.dll");
			if (not ntdll)
				throw Error::Win32Error(Win32::GetLastError(), "GetModuleHandleW() failed");

			m_mapViewOfSection = (MapViewOfSection)Win32::GetProcAddress(ntdll, "NtMapViewOfSection");
			if (not m_mapViewOfSection)
				throw Error::Win32Error(Win32::GetLastError(), "GetProcAddress() failed");

			m_querySystemInformation = (QuerySystemInformation)Win32::GetProcAddress(ntdll, "NtQuerySystemInformation");
			if (not m_querySystemInformation)
				throw Error::Win32Error(Win32::GetLastError(), "GetProcAddress() failed");

			m_duplicateObject = (DuplicateObject)Win32::GetProcAddress(ntdll, "NtDuplicateObject");
			if (not m_duplicateObject)
				throw Error::Win32Error(Win32::GetLastError(), "GetProcAddress() failed");

			m_queryObject = (QueryObject)Win32::GetProcAddress(ntdll, "NtQueryObject");
			if (not m_queryObject)
				throw Error::Win32Error(Win32::GetLastError(), "GetProcAddress() failed");
		}

		MapViewOfSection m_mapViewOfSection;
		QuerySystemInformation m_querySystemInformation;
		DuplicateObject m_duplicateObject;
		QueryObject m_queryObject;
	};
}

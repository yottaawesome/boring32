export module boring32:native_ntdll;
import boring32.shared;
import :error;
import :native_defs;

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
			if (auto lastError = Win32::GetLastError(); not ntdll)
				throw Error::Win32Error("GetModuleHandleW() failed", lastError);

			m_mapViewOfSection = (MapViewOfSection)Win32::GetProcAddress(ntdll, "NtMapViewOfSection");
			if (auto lastError = Win32::GetLastError(); not m_mapViewOfSection)
				throw Error::Win32Error("GetProcAddress() failed", lastError);

			m_querySystemInformation = (QuerySystemInformation)Win32::GetProcAddress(ntdll, "NtQuerySystemInformation");
			if (auto lastError = Win32::GetLastError(); not m_querySystemInformation)
				throw Error::Win32Error("GetProcAddress() failed", lastError);

			m_duplicateObject = (DuplicateObject)Win32::GetProcAddress(ntdll, "NtDuplicateObject");
			if (auto lastError = Win32::GetLastError(); not m_duplicateObject)
				throw Error::Win32Error("GetProcAddress() failed", lastError);

			m_queryObject = (QueryObject)Win32::GetProcAddress(ntdll, "NtQueryObject");
			if (auto lastError = Win32::GetLastError(); not m_queryObject)
				throw Error::Win32Error("GetProcAddress() failed", lastError);
		}

		MapViewOfSection m_mapViewOfSection;
		QuerySystemInformation m_querySystemInformation;
		DuplicateObject m_duplicateObject;
		QueryObject m_queryObject;
	};
}

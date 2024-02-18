export module boring32.native:ntdll;
import boring32.error;
import boring32.shared;
import :defs;

export namespace Boring32::Native
{
	class NTDLL final
	{
		public:
			NTDLL()
			{
				Map();
			}

			NTDLL(const NTDLL&) = delete;
			NTDLL(NTDLL&&) noexcept = delete;

		public:
			NTDLL& operator=(const NTDLL&) = delete;
			NTDLL& operator=(NTDLL&&) noexcept = delete;

		private:
			void Map()
			{
				Win32::HMODULE ntdll = Win32::GetModuleHandleW(L"ntdll.dll");
				if (!ntdll)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetModuleHandleW() failed", lastError);
				}

				m_mapViewOfSection = (MapViewOfSection)Win32::GetProcAddress(ntdll, "NtMapViewOfSection");
				if (!m_mapViewOfSection)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcAddress() failed", lastError);
				}

				m_querySystemInformation = (QuerySystemInformation)Win32::GetProcAddress(ntdll, "NtQuerySystemInformation");
				if (!m_querySystemInformation)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcAddress() failed", lastError);
				}

				m_duplicateObject = (DuplicateObject)Win32::GetProcAddress(ntdll, "NtDuplicateObject");
				if (!m_duplicateObject)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcAddress() failed", lastError);
				}

				m_queryObject = (QueryObject)Win32::GetProcAddress(ntdll, "NtQueryObject");
				if (!m_queryObject)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("GetProcAddress() failed", lastError);
				}
			}

		private:
			MapViewOfSection m_mapViewOfSection;
			QuerySystemInformation m_querySystemInformation;
			DuplicateObject m_duplicateObject;
			QueryObject m_queryObject;
	};
}

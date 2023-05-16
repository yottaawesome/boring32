module;

#include <source_location>;

export module boring32.win32:ntdll;
import :defs;
import boring32.error;
import <win32.hpp>;

export namespace Boring32::Win32
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

		private:
			MapViewOfSection m_mapViewOfSection;
			QuerySystemInformation m_querySystemInformation;
			DuplicateObject m_duplicateObject;
			QueryObject m_queryObject;
	};
}

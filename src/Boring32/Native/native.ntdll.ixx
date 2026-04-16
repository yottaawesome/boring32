export module boring32:native.ntdll;
import std;
import :win32;
import :error;
import :native.defs;

export namespace Boring32::Native
{
	class NativeModule
	{
	public:
		NativeModule(std::wstring_view moduleName)
		{
			hModule = Win32::HMODULE{ Win32::GetModuleHandleW(moduleName.data()) };
			if (not hModule)
				throw Error::Win32Error{ Win32::GetLastError(), "GetModuleHandleW() failed to load module" };
		}
		template<typename T>
		auto GetTypedProcAddress(this const NativeModule& self, std::string_view fnName) -> T
		{
			if (not self.hModule)
				throw Boring32::Error::Boring32Error("Module is null");
			auto fn = reinterpret_cast<T>(Win32::GetProcAddress(self.hModule, fnName.data()));
			return fn ? fn : throw Error::Win32Error{ Win32::GetLastError(), std::format("GetProcAddress() failed on function {}", fnName) };
		}
	private:
		Win32::HMODULE hModule = nullptr;
	};

	class NTDLL final
	{
	public:
		NTDLL()
		{
			Map();
		}

		NTDLL(const NTDLL&) = delete;
		NTDLL(NTDLL&&) noexcept = delete;

	private:
		void Map()
		{
			auto nativeModule = NativeModule{ L"ntdll.dll" };

			m_mapViewOfSection = nativeModule.GetTypedProcAddress<MapViewOfSection>("NtMapViewOfSection");
			m_querySystemInformation = nativeModule.GetTypedProcAddress<QuerySystemInformation>("NtQuerySystemInformation");
			m_duplicateObject = nativeModule.GetTypedProcAddress<DuplicateObject>("NtDuplicateObject");
			m_queryObject = nativeModule.GetTypedProcAddress<QueryObject>("NtQueryObject");
		}

		MapViewOfSection m_mapViewOfSection;
		QuerySystemInformation m_querySystemInformation;
		DuplicateObject m_duplicateObject;
		QueryObject m_queryObject;
	};
}

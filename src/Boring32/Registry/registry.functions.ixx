export module boring32:registry.functions;
import std;
import :win32;
import :error;
import :concepts;
import :async;

export namespace Boring32::Registry
{
	template<typename T>
	struct RegistryValueMap {};
	template<>
	struct RegistryValueMap<std::wstring> { static constexpr int Type = Win32::Winreg::_RRF_RT_REG_SZ; };
	template<>
	struct RegistryValueMap<Win32::DWORD> { static constexpr int Type = Win32::Winreg::_RRF_RT_REG_DWORD; };
	template<>
	struct RegistryValueMap<size_t> { static constexpr int Type = Win32::Winreg::_RRF_RT_REG_QWORD; };

	template<Concepts::OneOf<std::wstring, Win32::DWORD, size_t> T>
	T GetValue(const Win32::Winreg::HKEY key, const std::wstring& valueName)
	{
		if (not key)
			throw Error::Boring32Error("key is nullptr");

		T out;
		Win32::DWORD sizeInBytes = sizeof(out);
		const Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			RegistryValueMap<T>::Type,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(status, "RegGetValueW() failed");
		return out;
	}

	template<>
	std::wstring GetValue<std::wstring>(
		const Win32::Winreg::HKEY key,
		const std::wstring& valueName
	)
	{
		if (not key)
			throw Error::Boring32Error("m_key is null");

		std::wstring out;
		Win32::DWORD sizeInBytes = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
		Win32::LONG statusCode = Win32::Winreg::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			Win32::Winreg::_RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&sizeInBytes
		);
		if (statusCode != Win32::ErrorCodes::Success)
			throw Error::Win32Error(statusCode, "RegGetValueW() failed (1)");

		out.resize(sizeInBytes / sizeof(wchar_t), '\0');
		statusCode = Win32::Winreg::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			Win32::Winreg::_RRF_RT_REG_SZ,
			nullptr,
			&out[0],
			&sizeInBytes
		);
		if (statusCode != Win32::ErrorCodes::Success)
			throw Error::Win32Error(statusCode, "RegGetValueW() failed (2)");

		out.resize(sizeInBytes / sizeof(wchar_t));
		// Exclude terminating null
		if (not out.empty())
			out.pop_back();
		return out;
	}

	void GetValue(const Win32::Winreg::HKEY key, const std::wstring& valueName, std::wstring& out)
	{
		out = GetValue<std::wstring>(key, valueName);
	}

	void GetValue(const Win32::Winreg::HKEY key, const std::wstring& valueName, Win32::DWORD& out)
	{
		out = GetValue<Win32::DWORD>(key, valueName);
	}

	void GetValue(const Win32::Winreg::HKEY key, const std::wstring& valueName, size_t& out)
	{
		out = GetValue<size_t>(key, valueName);
	}

	template<typename T>
	void WriteValue(
		const Win32::Winreg::HKEY key,
		const std::wstring& valueName,
		const T& value
	)
	{
		if (not key)
			throw Error::Boring32Error("key is nullptr");

		Win32::LSTATUS status = Win32::Winreg::RegSetValueExW(
			key,
			valueName.c_str(),
			0,
			RegistryValueMap<T>::Type,
			reinterpret_cast<Win32::BYTE*>(const_cast<T*>(&value)),
			sizeof(value)
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(status, "RegSetValueExW() failed");
	}

	// This should probably be integrated into RegistryKey to enable safety checks
	// against calling it multiple times causing resource leaks
	void WatchKey(const Win32::Winreg::HKEY key, const Async::AnyEvent auto& eventToSignal)
	{
		if (not key)
			throw Error::Boring32Error("key cannot be nullptr");
		if (not eventToSignal)
			throw Error::Boring32Error("eventToSignal is not an initialised Event");

		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regnotifychangekeyvalue
		Win32::LSTATUS status = Win32::Winreg::RegNotifyChangeKeyValue(
			key,
			false,
			Win32::Winreg::_REG_NOTIFY_CHANGE_LAST_SET,
			eventToSignal.GetHandle(),
			true
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error("failed to watch registry key for changes", status);
	}

	void DeleteKeyAndSubkey(const Win32::Winreg::HKEY parent, const std::wstring& subkey)
	{
		if (not parent)
			throw Error::Boring32Error("Parent cannot be nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shdeletekeyw
		Win32::LSTATUS status = Win32::SHDeleteKeyW(parent, subkey.c_str());
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(Win32::GetLastError(), "SHDeleteKeyW() failed");
	}

	void DeleteSubkeys(const Win32::Winreg::HKEY parent, const std::wstring& subkey)
	{
		if (not parent)
			throw Error::Boring32Error("parent cannot be nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletetreew
		Win32::LSTATUS status = Win32::Winreg::RegDeleteTreeW(parent, subkey.c_str());
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error(Win32::GetLastError(), "RegDeleteTreeW() failed");
	}
}
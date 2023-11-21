export module boring32.registry:functions;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import boring32.async;

export namespace Boring32::Registry
{
	template<typename T, Win32::DWORD dataType>
	T GetValue(const Win32::HKEY key, const std::wstring& valueName)
	{
		if (!key)
			throw Error::Boring32Error("key is nullptr");

		T out;
		Win32::DWORD sizeInBytes = sizeof(out);
		const Win32::LSTATUS status = Win32::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			dataType,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error("RegGetValueW() failed", status);
		return out;
	}

	template<>
	std::wstring GetValue<std::wstring, Win32::_RRF_RT_REG_SZ>(
		const Win32::HKEY key, 
		const std::wstring& valueName
	)
	{
		if (!key)
			throw Error::Boring32Error("m_key is null");

		std::wstring out;
		Win32::DWORD sizeInBytes = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
		Win32::LONG statusCode = Win32::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			Win32::_RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&sizeInBytes
		);
		if (statusCode != Win32::ErrorCodes::Success)
			throw Error::Win32Error(
				"RegGetValueW() failed (1)",
				statusCode
			);

		out.resize(sizeInBytes / sizeof(wchar_t), '\0');
		statusCode = Win32::RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			Win32::_RRF_RT_REG_SZ,
			nullptr,
			&out[0],
			&sizeInBytes
		);
		if (statusCode != Win32::ErrorCodes::Success)
			throw Error::Win32Error(
				"RegGetValueW() failed (2)",
				statusCode
			);

		out.resize(sizeInBytes / sizeof(wchar_t));
		// Exclude terminating null
		if (!out.empty())
			out.pop_back();
		return out;
	}

	void GetValue(const Win32::HKEY key, const std::wstring& valueName, std::wstring& out)
	{
		out = GetValue<std::wstring, Win32::_RRF_RT_REG_SZ>(key, valueName);
	}

	void GetValue(const Win32::HKEY key, const std::wstring& valueName, Win32::DWORD& out)
	{
		out = GetValue<Win32::DWORD, Win32::_RRF_RT_REG_DWORD>(key, valueName);
	}

	void GetValue(const Win32::HKEY key, const std::wstring& valueName, size_t& out)
	{
		out = GetValue<Win32::DWORD, Win32::_RRF_RT_REG_QWORD>(key, valueName);
	}

	template<typename T>
	void WriteValue(
		const Win32::HKEY key,
		const std::wstring& valueName,
		const Win32::DWORD type,
		const T& value
	)
	{
		if (!key)
			throw Error::Boring32Error("key is nullptr");

		const Win32::LSTATUS status = Win32::RegSetValueExW(
			key,
			valueName.c_str(),
			0,
			type,
			reinterpret_cast<BYTE*>(const_cast<T*>(&value)),
			sizeof(value)
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error("RegSetValueExW() failed", status);
	}

	// This should probably be integrated into RegistryKey to enable safety checks
	// against calling it multiple times causing resource leaks
	void WatchKey(const Win32::HKEY key, const Async::Event& eventToSignal)
	{
		if (!key)
			throw Error::Boring32Error("key cannot be nullptr");
		if (!eventToSignal)
			throw Error::Boring32Error("eventToSignal is not an initialised Event");

		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regnotifychangekeyvalue
		Win32::LSTATUS status = Win32::RegNotifyChangeKeyValue(
			key,
			false,
			Win32::_REG_NOTIFY_CHANGE_LAST_SET,
			eventToSignal.GetHandle(),
			true
		);
		if (status != Win32::ErrorCodes::Success)
			throw Error::Win32Error("failed to watch registry key for changes", status);
	}

	void DeleteKeyAndSubkey(const Win32::HKEY parent, const std::wstring& subkey)
	{
		if (!parent)
			throw Error::Boring32Error("Parent cannot be nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-shdeletekeyw
		const Win32::LSTATUS status = Win32::SHDeleteKeyW(
			parent,
			subkey.c_str()
		);
		if (status != Win32::ErrorCodes::Success)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("SHDeleteKeyW() failed", lastError);
		}
	}

	void DeleteSubkeys(const Win32::HKEY parent, const std::wstring& subkey)
	{
		if (parent == nullptr)
			throw Error::Boring32Error("parent cannot be nullptr");

		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regdeletetreew
		const LSTATUS status = Win32::RegDeleteTreeW(
			parent,
			subkey.c_str()
		);
		if (status != Win32::ErrorCodes::Success)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("RegDeleteTreeW() failed", lastError);
		}
	}
}
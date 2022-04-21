module;

#include <string>
#include <stdexcept>
#include <Windows.h>

export module boring32.registry:functions;
import boring32.error;

export namespace Boring32::Registry
{
	template<typename T, DWORD dataType>
	T GetValue(const HKEY key, const std::wstring& valueName)
	{
		if (key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": key is nullptr");

		T out;
		DWORD sizeInBytes = sizeof(out);
		const LSTATUS status = RegGetValueW(
			key,
			nullptr,
			valueName.c_str(),
			dataType,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(std::source_location::current(), "RegGetValueW() failed", status);
		return out;
	}

	template<>
	std::wstring GetValue<std::wstring, RRF_RT_REG_SZ>(const HKEY key, const std::wstring& valueName);

	void GetValue(const HKEY key, const std::wstring& valueName, std::wstring& out);
	void GetValue(const HKEY key, const std::wstring& valueName, DWORD& out);
	void GetValue(const HKEY key, const std::wstring& valueName, size_t& out);

	template<typename T>
	void WriteValue(
		const HKEY key,
		const std::wstring& valueName,
		const DWORD type,
		const T& value
	)
	{
		if (key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": key is nullptr");

		const LSTATUS status = RegSetValueExW(
			key,
			valueName.c_str(),
			0,
			type,
			reinterpret_cast<BYTE*>(const_cast<T*>(&value)),
			sizeof(value)
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(std::source_location::current(), "RegSetValueExW() failed", status);
	}

	void DeleteKeyAndSubkey(const HKEY parent, const std::wstring& subkey);
	void DeleteSubkeys(const HKEY parent, const std::wstring& subkey);
}
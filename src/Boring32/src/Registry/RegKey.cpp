#include "pch.hpp"
#include "include/Registry/RegKey.hpp"
#include "include/Error/Error.hpp"

namespace Boring32::Registry
{
	std::shared_ptr<HKEY__> CreateRegKeyPtr(HKEY key)
	{
		return { key, [](const auto val) { RegCloseKey(val); } };
	}

	RegKey::~RegKey() 
	{
		Close();
	}

	RegKey::RegKey() : m_access(0) {}

	RegKey::RegKey(const HKEY key)
	:	m_key(CreateRegKeyPtr(key)),
		m_access(KEY_ALL_ACCESS)
	{}

	RegKey::RegKey(const HKEY key, const std::wstring& subkey)
	:	m_access(KEY_ALL_ACCESS)
	{
		InternalOpen(key, subkey);
	}

	RegKey::RegKey(const HKEY key, const std::wstring& subkey, const DWORD access)
	:	m_access(access)
	{
		InternalOpen(key, subkey);
	}

	RegKey::RegKey(
		const HKEY key, 
		const std::wstring& subkey, 
		const std::nothrow_t&
	) noexcept
	:	m_access(KEY_ALL_ACCESS)
	{
		Error::TryCatchLogToWCerr(
			[this, key = key, &subkey = subkey] { InternalOpen(key, subkey); }, 
			__FUNCSIG__
		);
	}

	RegKey::RegKey(
		const HKEY key, 
		const std::wstring& subkey, 
		const DWORD access,
		const std::nothrow_t&
	) noexcept
	:	m_access(access)
	{
		Error::TryCatchLogToWCerr(
			[this, key = key, &subkey = subkey] { InternalOpen(key, subkey); }, 
			__FUNCSIG__
		);
	}

	RegKey::RegKey(const RegKey& other)
	:	m_access(0)
	{
		Copy(other);
	}

	RegKey& RegKey::operator=(const RegKey& other)
	{
		return Copy(other);
	}

	RegKey& RegKey::Copy(const RegKey& other)
	{
		m_key = other.m_key;
		m_access = other.m_access;
		return *this;
	}

	RegKey& RegKey::operator=(const HKEY other)
	{
		m_key = CreateRegKeyPtr(other);
		return *this;
	}

	RegKey::RegKey(RegKey&& other) noexcept
	{
		Move(other);
	}

	RegKey& RegKey::operator=(RegKey&& other) noexcept
	{
		return Move(other);
	}
	
	RegKey::operator bool() const noexcept
	{
		return m_key != nullptr;
	}

	RegKey& RegKey::Move(RegKey& other) noexcept
	{
		m_key = std::move(other.m_key);
		m_access = other.m_access;
		return *this;
	}

	void RegKey::Close() noexcept
	{
		m_key = nullptr;
	}

	HKEY RegKey::GetKey() const noexcept
	{
		return m_key.get();
	}

	std::wstring RegKey::GetString(const std::wstring& valueName)
	{
		if (m_key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_key is null");

		DWORD sizeInBytes = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
		LONG statusCode = RegGetValueW(
			m_key.get(),
			nullptr,
			valueName.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&sizeInBytes
		);
		if (statusCode != ERROR_SUCCESS)
			throw Error::Win32Error(
				__FUNCSIG__ ": RegGetValueW() failed (1)",
				statusCode
			);

		std::wstring data(sizeInBytes / sizeof(wchar_t), '\0');
		statusCode = RegGetValueW(
			m_key.get(),
			nullptr,
			valueName.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			&data[0],
			&sizeInBytes
		);
		if (statusCode != ERROR_SUCCESS)
			throw Error::Win32Error(
				__FUNCSIG__ ": RegGetValueW() failed (2)",
				statusCode
			);

		data.resize(sizeInBytes / sizeof(wchar_t));
		// Exclude terminating null
		if (data.empty() == false)
			data.pop_back();

		return data;
	}

	void RegKey::WriteValue(
		const std::wstring& keyValueName,
		const std::wstring& keyValueValue
	)
	{
		if (m_key == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_key is null");

		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw
		const LSTATUS status = RegSetValueExW(
			m_key.get(),
			keyValueName.c_str(),
			0,
			REG_SZ,
			(LPBYTE)keyValueValue.c_str(),
			(DWORD)((keyValueValue.size() + 1) * sizeof(wchar_t))
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(__FUNCSIG__ ": RegSetValueExW() failed", status);
	}

	void RegKey::InternalOpen(const HKEY superKey, const std::wstring& subkey)
	{
		HKEY key = nullptr;
		const LSTATUS status = RegOpenKeyExW(
			superKey,
			subkey.c_str(),
			0,
			m_access,
			&key
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(__FUNCSIG__ ": failed to open registry key", status);
		m_key = CreateRegKeyPtr(key);
	}
}
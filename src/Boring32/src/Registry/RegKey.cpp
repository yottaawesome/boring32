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
		m_key = nullptr;
	}

	RegKey::RegKey() {}

	RegKey::RegKey(const HKEY key)
		: m_key(CreateRegKeyPtr(key))
	{}

	RegKey::RegKey(const HKEY key, const std::wstring& subkey)
	{
		InternalOpen(key, subkey);
	}

	RegKey::RegKey(const HKEY key, const std::wstring& subkey, const std::nothrow_t&) noexcept
	{
		Error::TryCatchLogToWCerr(
			[this, key = key, &subkey = subkey] { InternalOpen(key, subkey); }, 
			__FUNCSIG__
		);
	}

	RegKey::RegKey(const RegKey& other)
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
		m_key = std::move(other.m_key);
		return *this;
	}

	RegKey& RegKey::Move(RegKey& other) noexcept
	{
		return *this;
	}

	HKEY RegKey::GetKey() const noexcept
	{
		return m_key.get();
	}

	void RegKey::InternalOpen(const HKEY key, const std::wstring& subkey)
	{
		HKEY hKey = nullptr;
		LSTATUS status = RegOpenKeyExW(
			key,
			subkey.c_str(),
			0,
			KEY_READ,
			&hKey
		);
		if (status != ERROR_SUCCESS)
			throw Error::Win32Error(__FUNCSIG__ ": failed to open registry key", status);
		m_key = CreateRegKeyPtr(hKey);
	}
}
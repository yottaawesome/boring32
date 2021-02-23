#pragma once
#include <stdexcept>
#include <memory>
#include <Windows.h>

namespace Boring32::Registry
{
	class RegKey
	{
		public:
			virtual ~RegKey();
			RegKey();
			RegKey(const HKEY key, const std::wstring& subkey);
			RegKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access
			);
			RegKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept;
			RegKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access, 
				const std::nothrow_t&
			) noexcept;
			RegKey(const HKEY key);
			RegKey(const RegKey& other);
			RegKey(RegKey&& other) noexcept;

		public:
			virtual RegKey& operator=(const HKEY other);
			virtual RegKey& operator=(const RegKey& other);
			virtual RegKey& operator=(RegKey&& other) noexcept;
			virtual operator bool() const noexcept;

		public:
			virtual void Close() noexcept;
			virtual HKEY GetKey() const noexcept;
			virtual std::wstring GetString(const std::wstring& valueName);
			virtual void WriteValue(
				const std::wstring& keyValueName,
				const std::wstring& keyValueValue
			);

		protected:
			virtual RegKey& Copy(const RegKey& other);
			virtual RegKey& Move(RegKey& other) noexcept;
			virtual void InternalOpen(const HKEY key, const std::wstring& subkey);

		protected:
			std::shared_ptr<HKEY__> m_key;
			DWORD m_access;
	};
}
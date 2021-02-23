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
			RegKey(const HKEY key, const std::wstring& subkey, const std::nothrow_t&) noexcept;
			RegKey(const HKEY key);

			RegKey(const RegKey& other);
			virtual RegKey& operator=(const HKEY other);
			virtual RegKey& operator=(const RegKey& other);

			RegKey(RegKey&& other) noexcept;
			virtual RegKey& operator=(RegKey&& other) noexcept;

		public:
			virtual HKEY GetKey() const noexcept;
			virtual std::wstring GetString(const std::wstring& valueName);

		protected:
			virtual RegKey& Copy(const RegKey& other);
			virtual RegKey& Move(RegKey& other) noexcept;
			virtual void InternalOpen(const HKEY key, const std::wstring& subkey);

		protected:
			std::shared_ptr<HKEY__> m_key;
	};
}
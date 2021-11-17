module;

#include <stdexcept>
#include <memory>
#include <Windows.h>

export module boring32.registry.key;

export namespace Boring32::Registry
{
	class RegistryKey
	{
		public:
			virtual ~RegistryKey();
			RegistryKey();
			RegistryKey(const HKEY key, const std::wstring& subkey);
			RegistryKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access
			);
			RegistryKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept;
			RegistryKey(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access, 
				const std::nothrow_t&
			) noexcept;
			RegistryKey(const HKEY key);
			RegistryKey(const RegistryKey& other);
			RegistryKey(RegistryKey&& other) noexcept;

		public:
			virtual RegistryKey& operator=(const HKEY other);
			virtual RegistryKey& operator=(const RegistryKey& other);
			virtual RegistryKey& operator=(RegistryKey&& other) noexcept;
			virtual operator bool() const noexcept;

		public:
			virtual void Close() noexcept;
			virtual HKEY GetKey() const noexcept;
			virtual void GetValue(const std::wstring& valueName, std::wstring& out);
			virtual void GetValue(const std::wstring& valueName, DWORD& out);
			virtual void GetValue(const std::wstring& valueName, size_t& out);
			virtual void WriteValue(
				const std::wstring& valueName,
				const std::wstring& value
			);
			virtual void WriteValue(
				const std::wstring& valueName,
				const DWORD value
			);
			virtual void WriteValue(
				const std::wstring& valueName,
				const size_t value
			);
			virtual void Export(const std::wstring& path, const DWORD flags);

		protected:
			virtual RegistryKey& Copy(const RegistryKey& other);
			virtual RegistryKey& Move(RegistryKey& other) noexcept;
			virtual void InternalOpen(const HKEY key, const std::wstring& subkey);

		protected:
			std::shared_ptr<HKEY__> m_key;
			DWORD m_access;
	};
}
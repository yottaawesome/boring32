module;

#include <stdexcept>
#include <memory>
#include <Windows.h>

export module boring32.registry.key;

export namespace Boring32::Registry
{
	class Key
	{
		public:
			virtual ~Key();
			Key();
			Key(const HKEY key, const std::wstring& subkey);
			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access
			);
			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept;
			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access, 
				const std::nothrow_t&
			) noexcept;
			Key(const HKEY key);
			Key(const Key& other);
			Key(Key&& other) noexcept;

		public:
			virtual Key& operator=(const HKEY other);
			virtual Key& operator=(const Key& other);
			virtual Key& operator=(Key&& other) noexcept;
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
			virtual Key& Copy(const Key& other);
			virtual Key& Move(Key& other) noexcept;
			virtual void InternalOpen(const HKEY key, const std::wstring& subkey);
			virtual void InternalOpen(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept;

		protected:
			std::shared_ptr<HKEY__> m_key;
			DWORD m_access;
	};
}
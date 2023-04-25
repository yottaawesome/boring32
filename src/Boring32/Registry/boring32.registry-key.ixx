export module boring32.registry:key;
import :value;
import <stdexcept>;
import <memory>;
import <vector>;
import <utility>;
import <iostream>;
import <win32.hpp>;
import :functions;
import boring32.error;

namespace Boring32::Registry
{
	std::shared_ptr<HKEY__> CreateRegKeyPtr(HKEY key)
	{
		return { key, [](const auto val) { RegCloseKey(val); } };
	}
}

export namespace Boring32::Registry
{
	struct KeyValues
	{
		std::wstring Name;
		ValueTypes DataType = ValueTypes::None;
		DWORD DataSizeBytes = 0;
	};

	class Key
	{
		public:
			virtual ~Key()
			{
				Close();
			}

			Key() = default;
			Key(const HKEY key, const std::wstring& subkey)
				: m_access(KEY_ALL_ACCESS)
			{
				InternalOpen(key, subkey);
			}

			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access
			) 
				: m_access(access)
			{
				InternalOpen(key, subkey);
			}

			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept
				: m_access(KEY_ALL_ACCESS)
			{
				InternalOpen(key, subkey, std::nothrow);
			}

			Key(
				const HKEY key, 
				const std::wstring& subkey, 
				const DWORD access, 
				const std::nothrow_t&
			) noexcept
				: m_access(access)
			{
				InternalOpen(key, subkey, std::nothrow);
			}


			Key(const HKEY key)
				: m_key(CreateRegKeyPtr(key)),
				m_access(KEY_ALL_ACCESS)
			{}

			Key(const Key& other)
				: m_access(0)
			{
				Copy(other);
			}

			Key(Key&& other) noexcept
			{
				Move(other);
			}

		public:
			virtual Key& operator=(const HKEY other)
			{
				m_key = CreateRegKeyPtr(other);
				return *this;
			}

			virtual Key& operator=(const Key& other)
			{
				return Copy(other);
			}

			virtual Key& operator=(Key&& other) noexcept
			{
				return Move(other);
			}

			virtual operator bool() const noexcept
			{
				return m_key != nullptr;
			}

		public:
			virtual void Close() noexcept
			{
				m_key = nullptr;
			}

			virtual HKEY GetKey() const noexcept
			{
				return m_key.get();
			}

			virtual void GetValue(const std::wstring& valueName, std::wstring& out)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				Registry::GetValue(m_key.get(), valueName, out);
			}

			virtual void GetValue(const std::wstring& valueName, DWORD& out)
			{
				Registry::GetValue(m_key.get(), valueName, out);
			}

			virtual void GetValue(const std::wstring& valueName, size_t& out)
			{
				Registry::GetValue(m_key.get(), valueName, out);
			}

			virtual void WriteValue(
				const std::wstring& valueName,
				const std::wstring& value
			)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw
				const LSTATUS status = RegSetValueExW(
					m_key.get(),
					valueName.c_str(),
					0,
					REG_SZ,
					(BYTE*)value.c_str(),
					(DWORD)((value.size() + 1) * sizeof(wchar_t))
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("RegSetValueExW() failed", status);
			}

			virtual void WriteValue(
				const std::wstring& valueName,
				const DWORD value
			)
			{
				Registry::WriteValue(m_key.get(), valueName, REG_DWORD, value);
			}

			virtual void WriteValue(
				const std::wstring& valueName,
				const size_t value
			)
			{
				Registry::WriteValue(m_key.get(), valueName, REG_QWORD, value);
			}

			virtual void Export(const std::wstring& path, const DWORD flags)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				const LSTATUS status = RegSaveKeyExW(
					m_key.get(),
					path.c_str(),
					nullptr,
					REG_LATEST_FORMAT
				);
				if (status != ERROR_SUCCESS)
					throw Error::Win32Error("RegSaveKeyExW() failed", status);
			}

			virtual std::vector<KeyValues> GetValues()
			{
				if (!m_key)
				{
					throw Error::Boring32Error("Key not initialised.");
				}

				// See: https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
				constexpr unsigned maxValueNameCharacterLength = 32767 / sizeof(wchar_t);
				std::wstring valueNameBuffer(maxValueNameCharacterLength, '\0');
				std::vector<KeyValues> values;
				// Essentially, this works by requesting the value at the index, which is incremented
				// per successful iteration. Once the end is reached, RegEnumValueW() will fail with
				// ERROR_NO_MORE_ITEMS, at which point we can terminate the loop. Note that the order 
				// of the enumeration is unspecified and will probably differ to what appears in 
				// Registry Editor.
				for (DWORD index = 0;; index++)
				{
					DWORD valueNameCharacterLength = maxValueNameCharacterLength;
					KeyValues valueToAdd{};
					// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluew
					const DWORD status = RegEnumValueW(
						m_key.get(),
						index,
						&valueNameBuffer[0],
						&valueNameCharacterLength,
						0,
						reinterpret_cast<DWORD*>(&valueToAdd.DataType),
						nullptr,
						&valueToAdd.DataSizeBytes
					);
					if (status == ERROR_NO_MORE_ITEMS)
					{
						break;
					}
					if (status != ERROR_SUCCESS)
					{
						throw Error::Win32Error("RegEnumValueW() failed.", status);
					}
					valueToAdd.Name = std::wstring(
						valueNameBuffer.begin(),
						valueNameBuffer.begin() + valueNameCharacterLength
					);
					values.push_back(std::move(valueToAdd));
				}

				return values;
			}

		protected:
			virtual Key& Copy(const Key& other)
			{
				m_key = other.m_key;
				m_access = other.m_access;
				return *this;
			}

			virtual Key& Move(Key& other) noexcept
			{
				m_key = std::move(other.m_key);
				m_access = other.m_access;
				return *this;
			}

			virtual void InternalOpen(
				const HKEY superKey,
				const std::wstring& subkey
			)
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
					throw Error::Win32Error("failed to open registry key", status);
				m_key = CreateRegKeyPtr(key);
			}

			virtual void InternalOpen(
				const HKEY key, 
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept try
			{
				InternalOpen(key, subkey);
			}
			catch (const std::exception& ex)
			{
				std::wcerr << ex.what() << std::endl;
			}

		protected:
			std::shared_ptr<HKEY__> m_key;
			DWORD m_access = 0;
	};
}
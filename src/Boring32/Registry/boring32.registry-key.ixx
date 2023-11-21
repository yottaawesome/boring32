export module boring32.registry:key;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import :functions;
import :value;

export namespace Boring32::Registry
{
	struct KeyValues
	{
		std::wstring Name;
		ValueTypes DataType = ValueTypes::None;
		Win32::DWORD DataSizeBytes = 0;
	};

	class Key final
	{
		public:
			~Key() = default;
			Key() = default;
			Key(const Key& other) = default;
			Key(Key&& other) noexcept = default;
			Key& operator=(const Key& other) = default;
			Key& operator=(Key&& other) noexcept = default;

		public:
			Key(const Win32::HKEY key, const std::wstring& subkey)
				: m_access(Win32::_KEY_ALL_ACCESS)
			{
				InternalOpen(key, subkey);
			}

			Key(
				const Win32::HKEY key,
				const std::wstring& subkey, 
				const Win32::DWORD access
			) : m_access(access)
			{
				InternalOpen(key, subkey);
			}

			Key(
				const Win32::HKEY key,
				const std::wstring& subkey, 
				const std::nothrow_t&
			) noexcept
				: m_access(Win32::_KEY_ALL_ACCESS)
			{
				InternalOpen(key, subkey, std::nothrow);
			}

			Key(
				const Win32::HKEY key,
				const std::wstring& subkey, 
				const Win32::DWORD access,
				const std::nothrow_t&
			) noexcept
				: m_access(access)
			{
				InternalOpen(key, subkey, std::nothrow);
			}


			Key(const HKEY key)
				: m_key(CreateRegKeyPtr(key)),
				m_access(Win32::_KEY_ALL_ACCESS)
			{}

		public:
			Key& operator=(const HKEY other)
			{
				m_key = CreateRegKeyPtr(other);
				return *this;
			}

			operator bool() const noexcept
			{
				return m_key != nullptr;
			}

		public:
			void Close() noexcept
			{
				m_key = nullptr;
			}

			HKEY GetKey() const noexcept
			{
				return m_key.get();
			}

			void GetValue(const std::wstring& valueName, std::wstring& out)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				Registry::GetValue(m_key.get(), valueName, out);
			}

			void GetValue(const std::wstring& valueName, DWORD& out)
			{
				Registry::GetValue(m_key.get(), valueName, out);
			}

			void GetValue(const std::wstring& valueName, size_t& out)
			{
				Registry::GetValue(m_key.get(), valueName, out);
			}

			void WriteValue(
				const std::wstring& valueName,
				const std::wstring& value
			)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw
				const Win32::LSTATUS status = Win32::RegSetValueExW(
					m_key.get(),
					valueName.c_str(),
					0,
					(Win32::DWORD)ValueTypes::String,
					(Win32::BYTE*)value.c_str(),
					(Win32::DWORD)((value.size() + 1) * sizeof(wchar_t))
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("RegSetValueExW() failed", status);
			}

			void WriteValue(
				const std::wstring& valueName,
				const Win32::DWORD value
			)
			{
				Registry::WriteValue(m_key.get(), valueName, (Win32::DWORD)Win32::ValueTypes::DWord, value);
			}

			void WriteValue(
				const std::wstring& valueName,
				const size_t value
			)
			{
				Registry::WriteValue(m_key.get(), valueName, (Win32::DWORD)Win32::ValueTypes::QWord, value);
			}

			void Export(const std::wstring& path, const Win32::DWORD flags)
			{
				if (!m_key)
					throw Error::Boring32Error("m_key is null");

				const Win32::LSTATUS status = Win32::RegSaveKeyExW(
					m_key.get(),
					path.c_str(),
					nullptr,
					Win32::_REG_LATEST_FORMAT
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error("RegSaveKeyExW() failed", status);
			}

			std::vector<KeyValues> GetValues()
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
				for (Win32::DWORD index = 0;; index++)
				{
					Win32::DWORD valueNameCharacterLength = maxValueNameCharacterLength;
					KeyValues valueToAdd{};
					// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluew
					const Win32::DWORD status = Win32::RegEnumValueW(
						m_key.get(),
						index,
						&valueNameBuffer[0],
						&valueNameCharacterLength,
						0,
						reinterpret_cast<Win32::DWORD*>(&valueToAdd.DataType),
						nullptr,
						&valueToAdd.DataSizeBytes
					);
					if (status == Win32::ErrorCodes::NoMoreItems)
					{
						break;
					}
					if (status != Win32::ErrorCodes::Success)
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

			unsigned GetSubkeyCount()
			{
				Win32::DWORD subkeys;
				// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryinfokeyw
				const Win32::LSTATUS status = Win32::RegQueryInfoKeyW(
					m_key.get(),
					nullptr,
					nullptr,
					nullptr,
					&subkeys,
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					nullptr,
					nullptr
				);
				if (status)
					throw Error::Win32Error(
						"Failed to open registry key",
						status
					);
				return subkeys;
			}

			bool IsPredefinedKey() const noexcept
			{
				return IsPredefinedKey(m_key.get());
			}

			static bool IsPredefinedKey(Win32::HKEY const key) noexcept
			{
				if (key == Win32::_HKEY_CLASSES_ROOT)
					return true;
				if (key == Win32::_HKEY_CURRENT_CONFIG)
					return true;
				if (key == Win32::_HKEY_CURRENT_USER)
					return true;
				if (key == Win32::_HKEY_LOCAL_MACHINE)
					return true;
				if (key == Win32::_HKEY_USERS)
					return true;
				return false;
			}

		private:
			void InternalOpen(
				const Win32::HKEY superKey,
				const std::wstring& subkey
			)
			{
				Win32::HKEY key = nullptr;
				const Win32::LSTATUS status = Win32::RegOpenKeyExW(
					superKey,
					subkey.c_str(),
					0,
					m_access,
					&key
				);
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error(
						"Failed to open registry key", 
						status
					);
				m_key = CreateRegKeyPtr(key);
			}

			void InternalOpen(
				const Win32::HKEY key,
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

			static std::shared_ptr<Win32::HKEY__> CreateRegKeyPtr(Win32::HKEY key)
			{
				return {
					key,
					[](const Win32::HKEY val)
					{
						if (!Key::IsPredefinedKey(val))
							Win32::RegCloseKey(val);
					}
				};
			}

		private:
			std::shared_ptr<Win32::HKEY__> m_key;
			Win32::DWORD m_access = 0;
	};
}
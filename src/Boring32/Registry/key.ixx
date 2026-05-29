export module boring32:registry.key;
import std;
import :win32;
import :error;
import :registry.functions;
import :registry.value;

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
		Key() = default;
		Key(const Key& other) = default;
		Key(Key&& other) noexcept = default;
		auto operator=(const Key& other) -> Key& = default;
		auto operator=(Key&& other) noexcept -> Key& = default;

		Key(const Win32::Winreg::HKEY key, const std::wstring& subkey)
			: m_access(Win32::Winreg::_KEY_ALL_ACCESS)
		{
			InternalOpen(key, subkey);
		}

		Key(
			const Win32::Winreg::HKEY key,
			const std::wstring& subkey, 
			const Win32::DWORD access
		) : m_access(access)
		{
			InternalOpen(key, subkey);
		}

		Key(
			const Win32::Winreg::HKEY key,
			const std::wstring& subkey, 
			const std::nothrow_t&
		) noexcept
			: m_access(Win32::Winreg::_KEY_ALL_ACCESS)
		{
			TryInternalOpen(key, subkey);
		}

		Key(
			const Win32::Winreg::HKEY key,
			const std::wstring& subkey, 
			const Win32::DWORD access,
			const std::nothrow_t&
		) noexcept
			: m_access(access)
		{
			TryInternalOpen(key, subkey);
		}

		Key(const Win32::Winreg::HKEY key)
			: m_key(CreateRegKeyPtr(key)),
			m_access(Win32::Winreg::_KEY_ALL_ACCESS)
		{}

		auto operator=(const Win32::Winreg::HKEY other) -> Key&
		{
			m_key = CreateRegKeyPtr(other);
			return *this;
		}

		operator bool() const noexcept
		{
			return m_key != nullptr;
		}

		void Close() noexcept
		{
			m_key = nullptr;
		}

		auto GetKey() const noexcept -> Win32::Winreg::HKEY
		{
			return m_key.get();
		}

		void GetValue(const std::wstring& valueName, std::wstring& out)
		{
			if (not m_key)
				throw Error::Boring32Error{ "m_key is null" };

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

		void WriteValue(const std::wstring& valueName, const std::wstring& value)
		{
			if (not m_key)
				throw Error::Boring32Error{ "m_key is null" };

			// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regsetvalueexw
			auto status = Win32::LSTATUS{
				Win32::Winreg::RegSetValueExW(
					m_key.get(),
					valueName.c_str(),
					0,
					static_cast<Win32::DWORD>(ValueTypes::String),
					reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(value.c_str())),
					static_cast<Win32::DWORD>((value.size() + 1) * sizeof(wchar_t))
				) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{static_cast<Win32::DWORD>(status), "RegSetValueExW() failed"};
		}

		void WriteValue(const std::wstring& valueName, Win32::DWORD value)
		{
			Registry::WriteValue(m_key.get(), valueName, value);
		}

		void WriteValue(const std::wstring& valueName, size_t value)
		{
			Registry::WriteValue(m_key.get(), valueName, value);
		}

		void Export(const std::wstring& path, Win32::DWORD flags)
		{
			if (not m_key)
				throw Error::Boring32Error{ "m_key is null" };

			auto status = Win32::LSTATUS{ 
				Win32::Winreg::RegSaveKeyExW(
					m_key.get(),
					path.c_str(),
					nullptr,
					Win32::Winreg::_REG_LATEST_FORMAT
				) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{static_cast<Win32::DWORD>(status), "RegSaveKeyExW() failed"};
		}

		auto GetValues() -> std::vector<KeyValues>
		{
			if (not m_key)
				throw Error::Boring32Error{ "Key not initialised." };

			// See: https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
			constexpr unsigned maxValueNameCharacterLength = 32767 / sizeof(wchar_t);
			auto valueNameBuffer = std::wstring(maxValueNameCharacterLength, '\0');
			auto values = std::vector<KeyValues>{};
			// Essentially, this works by requesting the value at the index, which is incremented
			// per successful iteration. Once the end is reached, RegEnumValueW() will fail with
			// ERROR_NO_MORE_ITEMS, at which point we can terminate the loop. Note that the order 
			// of the enumeration is unspecified and will probably differ to what appears in 
			// Registry Editor.
			for (auto index = Win32::DWORD{};; index++)
			{
				auto valueNameCharacterLength = Win32::DWORD{maxValueNameCharacterLength};
				auto valueToAdd = KeyValues{};
				// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regenumvaluew
				auto status = Win32::LSTATUS{
					Win32::Winreg::RegEnumValueW(
						m_key.get(),
						index,
						&valueNameBuffer[0],
						&valueNameCharacterLength,
						0,
						reinterpret_cast<Win32::DWORD*>(&valueToAdd.DataType),
						nullptr,
						&valueToAdd.DataSizeBytes
					)};
				if (status == Win32::ErrorCodes::NoMoreItems)
				{
					break;
				}
				if (status != Win32::ErrorCodes::Success)
					throw Error::Win32Error{static_cast<Win32::DWORD>(status), "RegEnumValueW() failed."};
				valueToAdd.Name = std::wstring(
					valueNameBuffer.begin(),
					valueNameBuffer.begin() + valueNameCharacterLength
				);
				values.push_back(std::move(valueToAdd));
			}

			return values;
		}

		auto GetSubkeyCount() -> unsigned
		{
			auto subkeys = Win32::DWORD{};
			// https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryinfokeyw
			auto status = Win32::LSTATUS{ 
				Win32::Winreg::RegQueryInfoKeyW(
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
				)};
			if (status)
				throw Error::Win32Error{static_cast<Win32::DWORD>(status), "Failed to open registry key"};
			return subkeys;
		}

		auto IsPredefinedKey() const noexcept -> bool
		{
			return IsPredefinedKey(m_key.get());
		}

		static auto IsPredefinedKey(Win32::Winreg::HKEY const key) noexcept -> bool
		{
			if (key == Win32::Winreg::Keys::HKCR)
				return true;
			if (key == Win32::Winreg::Keys::HKCC)
				return true;
			if (key == Win32::Winreg::Keys::HKCU)
				return true;
			if (key == Win32::Winreg::Keys::HKLM)
				return true;
			if (key == Win32::Winreg::Keys::HKU)
				return true;
			return false;
		}

	private:
		void InternalOpen(Win32::Winreg::HKEY superKey, const std::wstring& subkey)
		{
			auto key = Win32::Winreg::HKEY{};
			auto status = Win32::LSTATUS{ 
				Win32::Winreg::RegOpenKeyExW(
					superKey,
					subkey.c_str(),
					0,
					m_access,
					&key
				) };
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error{static_cast<Win32::DWORD>(status), "Failed to open registry key"};
			m_key = CreateRegKeyPtr(key);
		}

		void TryInternalOpen(Win32::Winreg::HKEY key, const std::wstring& subkey) noexcept 
		try
		{
			InternalOpen(key, subkey);
		}
		catch (const std::exception& ex)
		{
			std::wcerr << ex.what() << std::endl;
		}

		static auto CreateRegKeyPtr(Win32::Winreg::HKEY key) -> std::shared_ptr<Win32::Winreg::HKEY__>
		{
			return {
				key,
				[](const Win32::Winreg::HKEY val)
				{
					if (not Key::IsPredefinedKey(val))
						Win32::Winreg::RegCloseKey(val);
				}
			};
		}

		std::shared_ptr<Win32::Winreg::HKEY__> m_key;
		Win32::DWORD m_access = 0;
	};
}
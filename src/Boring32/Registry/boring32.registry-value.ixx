export module boring32.registry:value;
import <string>;
import <tuple>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Registry
{
	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
	enum class ValueTypes : DWORD
	{
		None = REG_NONE,
		Binary = REG_BINARY,
		DWord = REG_DWORD,
		DWordLittleEndian = REG_DWORD_LITTLE_ENDIAN,
		DWordBigEndian = REG_DWORD_BIG_ENDIAN,
		ExpandableString = REG_EXPAND_SZ,
		Link = REG_LINK,
		MultiString = REG_MULTI_SZ,
		QWord = REG_QWORD,
		QWordLittleEndian = REG_QWORD_LITTLE_ENDIAN,
		String = REG_SZ,
	};

	template<ValueTypes V, typename T>
	struct KeyValuePair
	{
		ValueTypes V;
		using LanguageType = T;
	};
	template<>
	struct KeyValuePair<ValueTypes::String, std::string> { void Blah() {} };

	

	template<typename T>
	class Value
	{
		public:
			virtual ~Value() { }
			Value(const Value& other) = default;
			Value(Value&& other) noexcept = default;
			Value(
				const std::wstring& path,
				const std::wstring& valueName,
				const T value
			)
				: m_path(path),
				m_valueName(valueName),
				m_value(std::move(value))
			{
			}
			
		public:
			virtual Value& operator=(const Value& other) = default;
			virtual Value& operator=(Value&& other) noexcept = default;

		public:
			virtual const std::wstring& GetPath() const noexcept { return m_path; }
			virtual const std::wstring& GetValueName() const noexcept { return m_valueName; }
			virtual const T& GetValue() const noexcept { return m_value; }

		protected:
			std::wstring m_path;
			std::wstring m_valueName;
			T m_value;
	};

	template <size_t N>
	struct FixedString
	{
		wchar_t buf[N]{};
		constexpr FixedString(const wchar_t(&arg)[N]) noexcept
		{
			//buf = arg;
			for (unsigned i = 0; i < N; i++)
				buf[i] = arg[i];
		}

		constexpr operator const wchar_t* () const noexcept
		{
			return buf;
		}
	};
	template<size_t N>
	FixedString(wchar_t const (&)[N]) -> FixedString<N>;

	template <typename T>
	constexpr bool always_false = std::false_type::value;

	template<ValueTypes TValueType, HKEY TParentKey, FixedString TSubKey, FixedString TValueName>
	class RegistryValue
	{
		static constexpr const wchar_t* SubKey = TSubKey;
		static constexpr const wchar_t* ValueName = TValueName;
		using T = RegistryValue<TValueType, TParentKey, TSubKey, TValueName>;

		public:
			static constexpr const wchar_t* GetSubKey() noexcept
			{
				return SubKey;
			}

			static constexpr const wchar_t* GetValueName() noexcept
			{
				return ValueName;
			}
			
			static constexpr ValueTypes GetValueType() noexcept
			{
				return TValueType;
			}

			static auto Read()
			{
				if constexpr (TValueType == ValueTypes::DWord)
				{
					DWORD out;
					DWORD sizeInBytes = sizeof(out);
					LSTATUS status = RegGetValueW(
						TParentKey,
						SubKey,
						ValueName,
						RRF_RT_REG_DWORD,
						nullptr,
						&out,
						&sizeInBytes
					);
					if (status != ERROR_SUCCESS)
						throw Error::Win32Error("RegGetValueW() failed", status);
					return out;
				}
				else
				{
					static_assert(always_false<T>, "Unhandled value");
				}
			}
	};
}
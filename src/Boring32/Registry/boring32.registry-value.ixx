export module boring32.registry:value;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import boring32.strings;

export namespace Boring32::Registry
{
	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types
	using ValueTypes = Win32::ValueTypes;

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

	template <typename T>
	constexpr bool always_false = std::false_type::value;

	template<ValueTypes T, typename TInvokeResult>
	struct OP
	{
		auto operator()()
		{
			if constexpr (!std::is_same_v<TInvokeResult, void>)
			{
				if constexpr (T == ValueTypes::DWord)
				{
					return 1ul;
				}
				else if constexpr (T == ValueTypes::String)
				{
					return std::wstring{};
				}
			}
		}
	};

	template<auto F, ValueTypes V>
	concept CheckInvocable = requires(OP<V, std::invoke_result_t<decltype(F)>> op)
	{
		F();
		op();
		{F()} -> std::convertible_to<std::invoke_result_t<OP<V, std::invoke_result_t<decltype(F)>>>>;
	};

	template<Win32::HKEY TParentKey, Strings::FixedStringW TSubKey, Strings::FixedStringW TValueName, ValueTypes TValueType, auto TDefaultValue = [] {} >
		requires CheckInvocable<TDefaultValue, TValueType> // not really required due to the static_asserts below
	class RegistryValue
	{
		using T = RegistryValue<TParentKey, TSubKey, TValueName, TValueType>;
		using ReturnDefaultType = std::invoke_result_t<decltype(TDefaultValue)>;
		static constexpr const wchar_t* SubKey = TSubKey;
		static constexpr const wchar_t* ValueName = TValueName;
		static constexpr bool ThrowOnError = std::is_same_v<ReturnDefaultType, void>;
		static constexpr bool ReturnDefault = not ThrowOnError;

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

			static Win32::DWORD ReadDWord()
				requires (TValueType == ValueTypes::DWord)
			{
				Win32::DWORD out;
				Win32::DWORD sizeInBytes = sizeof(out);
				Win32::LSTATUS status = Win32::RegGetValueW(
					TParentKey,
					SubKey,
					ValueName,
					Win32::_RRF_RT_REG_DWORD,
					nullptr,
					&out,
					&sizeInBytes
				);
				if (status != Win32::ErrorCodes::Success)
				{
					if constexpr (ReturnDefault)
						return TDefaultValue();
					throw Error::Win32Error("RegGetValueW() failed", status);
				}
				return out;
			}

			static uint64_t ReadQWord()
				requires (TValueType == ValueTypes::QWord)
			{
				Win32::DWORD out;
				Win32::DWORD sizeInBytes = sizeof(out);
				Win32::LSTATUS status = Win32::RegGetValueW(
					TParentKey,
					SubKey,
					ValueName,
					Win32::_RRF_RT_REG_QWORD,
					nullptr,
					&out,
					&sizeInBytes
				);
				if (status != Win32::ErrorCodes::Success)
				{
					if constexpr (ReturnDefault)
						return TDefaultValue();
					throw Error::Win32Error("RegGetValueW() failed", status);
				}
				return out;
			}

			static std::wstring ReadString()
				requires (TValueType == ValueTypes::String)
			{
				Win32::DWORD sizeInBytes = 0;
				// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
				Win32::LSTATUS statusCode = Win32::RegGetValueW(
					TParentKey,
					SubKey,
					ValueName,
					Win32::_RRF_RT_REG_SZ,
					nullptr,
					nullptr,
					&sizeInBytes
				);
				if (statusCode != Win32::ErrorCodes::Success)
				{
					if constexpr (ReturnDefault)
						return TDefaultValue();
					throw Error::Win32Error("RegGetValueW() failed (1)", statusCode);
				}

				std::wstring out(sizeInBytes / sizeof(wchar_t), '\0');
				statusCode = Win32::RegGetValueW(
					TParentKey,
					SubKey,
					ValueName,
					Win32::_RRF_RT_REG_SZ,
					nullptr,
					&out[0],
					&sizeInBytes
				);
				if (statusCode != Win32::ErrorCodes::Success)
				{
					if constexpr (ReturnDefault)
						return TDefaultValue();
					throw Error::Win32Error("RegGetValueW() failed (2)", statusCode);
				}

				out.resize(sizeInBytes / sizeof(wchar_t));
				// Exclude terminating null
				if (!out.empty())
					out.pop_back();
				return out;
			}

			static auto Read()
			{
				if constexpr (TValueType == ValueTypes::DWord)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, Win32::DWORD>, "Return type from default lambda should be DWORD or convertible to DWORD.");
					return ReadDWord();
				}
				else if constexpr (TValueType == ValueTypes::String)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, std::wstring>, "Return type from default lambda should be wstring or convertible to wstring.");
					return ReadString();
				}
				else if constexpr (TValueType == ValueTypes::QWord)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, uint64_t>, "Return type from default lambda should be wstring or convertible to uint64_t.");
					return ReadQWord();
				}
				else
				{
					static_assert(always_false<T>, "Unhandled value");
				}
			}
	};
}
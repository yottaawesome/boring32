export module boring32.registry:registryvalue;
import std;
import std.compat;
import boring32.win32;
import boring32.error;
import boring32.strings;
import boring32.concepts;

namespace Boring32::Registry
{
	using ValueTypes = Win32::ValueTypes;

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
				else if constexpr (T == ValueTypes::QWord)
				{
					return 1ull;
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

	template<ValueTypes TValueType, typename TValue, bool ThrowOnError = true>
	void SetValue(Win32::HKEY parentKey, const wchar_t* subKey, const wchar_t* valueName, const TValue& value)
	{
		Win32::HKEY key = nullptr;
		Win32::LSTATUS status = Win32::RegOpenKeyExW(
			parentKey,
			subKey,
			0,
			Win32::_KEY_SET_VALUE,
			&key
		);
		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("Failed to open registry key", status);

		if constexpr (TValueType == ValueTypes::DWord or TValueType == ValueTypes::QWord)
		{
			status = Win32::RegSetValueExW(
				key,
				valueName,
				0,
				Win32::DWORD(TValueType),
				reinterpret_cast<BYTE*>(const_cast<TValue*>(&value)),
				sizeof(value)
			);
		}
		else if constexpr (TValueType == ValueTypes::String)
		{
			static_assert(std::convertible_to<TValue, std::wstring_view>);
			std::wstring_view str{ value };
			status = Win32::RegSetValueExW(
				key,
				valueName,
				0,
				Win32::DWORD(TValueType),
				reinterpret_cast<BYTE*>(const_cast<wchar_t*>(str.data())),
				static_cast<DWORD>((str.size() + 1) * sizeof(wchar_t))
			);
		}
		else
		{
			static_assert(Concepts::AlwaysFalse<TValue>);
		}
		Win32::RegCloseKey(key);

		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("RegSetValueExW() failed", status);
	}

	template<ValueTypes TValueType>
	struct ValueTraits : std::false_type {};

	template<>
	struct ValueTraits<ValueTypes::DWord>
	{
		using IntegralType = Win32::DWORD;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::DWord);
		static constexpr size_t Size = sizeof(IntegralType);
	};
	template<>
	struct ValueTraits<ValueTypes::QWord>
	{
		using IntegralType = uint64_t;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::QWord);
		static constexpr size_t Size = sizeof(IntegralType);
	};
	template<>
	struct ValueTraits<ValueTypes::String>
	{
		using IntegralType = std::wstring;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::String);
	};

	template<ValueTypes TValueType, bool ThrowOnError = true, Concepts::NullPtrOrInvocable auto DefaultValue = nullptr>
	auto ReadValue(Win32::HKEY ParentKey, const wchar_t* SubKey, const wchar_t* ValueName)
	{
		if constexpr (TValueType == ValueTypes::DWord or TValueType == ValueTypes::QWord)
		{
			using TraitType = ValueTraits<TValueType>;
			using IntegralType = typename TraitType::IntegralType;
			IntegralType out{};
			
			Win32::DWORD sizeInBytes = sizeof(out);
			Win32::LSTATUS status = Win32::RegGetValueW(
				ParentKey,
				SubKey,
				ValueName,
				TraitType::RawType,
				nullptr,
				&out,
				&sizeInBytes
			);
			if (status != Win32::ErrorCodes::Success)
			{
				if constexpr (ThrowOnError)
					throw Error::Win32Error("RegGetValueW() failed", status);
				else if (std::invocable<decltype(DefaultValue)>)
					return static_cast<IntegralType>(DefaultValue());
				else
					return IntegralType{};
			}
			return out;
		}
		else if constexpr (TValueType == ValueTypes::String)
		{
			Win32::DWORD sizeInBytes = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
			Win32::LSTATUS status = Win32::RegGetValueW(
				ParentKey,
				SubKey,
				ValueName,
				Win32::_RRF_RT_REG_SZ,
				nullptr,
				nullptr,
				&sizeInBytes
			);
			if (status != Win32::ErrorCodes::Success)
			{
				if constexpr (ThrowOnError)
					throw Error::Win32Error("RegGetValueW() failed", status);
				else if (std::invocable<decltype(DefaultValue)>)
					return std::wstring{ DefaultValue() };
				else
					return std::wstring{};
			}

			std::wstring out(sizeInBytes / sizeof(wchar_t), '\0');
			status = Win32::RegGetValueW(
				ParentKey,
				SubKey,
				ValueName,
				Win32::_RRF_RT_REG_SZ,
				nullptr,
				&out[0],
				&sizeInBytes
			);
			if (status != Win32::ErrorCodes::Success)
			{
				if constexpr (ThrowOnError)
					throw Error::Win32Error("RegGetValueW() failed", status);
				else if (std::invocable<decltype(DefaultValue)>)
					return std::wstring{ DefaultValue() };
				else
					return DefaultValue();
			}

			out.resize(sizeInBytes / sizeof(wchar_t));
			// Exclude terminating null
			if (!out.empty())
				out.pop_back();
			return out;
		}
		else
		{
			static_assert(Concepts::AlwaysFalse<TValueType>);
		}
	}

	template<bool ThrowOnError = true, Concepts::NullPtrOrInvocable auto DefaultValue = nullptr>
	Win32::DWORD ReadDWord(Win32::HKEY ParentKey, const wchar_t* SubKey, const wchar_t* ValueName) noexcept(not ThrowOnError)
	{
		static_assert(
			ThrowOnError 
			or (
				not std::is_null_pointer_v<decltype(DefaultValue)>
				and std::is_invocable_v<decltype(DefaultValue)>
				and std::is_convertible_v<std::invoke_result_t<decltype(DefaultValue)>, DWORD>
			),
			"Either ThrowOnError must be true or DefaultValue must be an invocable returning a DWORD"
		);

		Win32::DWORD out;
		Win32::DWORD sizeInBytes = sizeof(out);
		Win32::LSTATUS status = Win32::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::_RRF_RT_REG_DWORD,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else if (std::invocable<decltype(DefaultValue)>)
				return Win32::DWORD{ DefaultValue() };
			else
				return 0;
		}
		return out;
	}

	template<bool ThrowOnError = true, auto DefaultValue = [] { return 0ull; } >
	uint64_t ReadQWord(Win32::HKEY ParentKey, const wchar_t* SubKey, const wchar_t* ValueName) noexcept(not ThrowOnError)
	{
		static_assert(ThrowOnError or std::convertible_to<std::invoke_result_t<decltype(DefaultValue)>, uint64_t>);

		uint64_t out;
		Win32::DWORD sizeInBytes = sizeof(out);
		Win32::LSTATUS status = Win32::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::_RRF_RT_REG_QWORD,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else
				return DefaultValue();
		}
		return out;
	}

	template<bool ThrowOnError = true, auto DefaultValue = [] { return 0ull; } >
	std::wstring ReadString(Win32::HKEY ParentKey, const wchar_t* SubKey, const wchar_t* ValueName) noexcept(not ThrowOnError)
	{
		static_assert(ThrowOnError or std::convertible_to<std::invoke_result_t<decltype(DefaultValue)>, std::wstring>);

		Win32::DWORD sizeInBytes = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
		Win32::LSTATUS status = Win32::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::_RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else
				return DefaultValue();
		}

		std::wstring out(sizeInBytes / sizeof(wchar_t), '\0');
		status = Win32::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::_RRF_RT_REG_SZ,
			nullptr,
			&out[0],
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else
				return DefaultValue();
		}

		out.resize(sizeInBytes / sizeof(wchar_t));
		// Exclude terminating null
		if (!out.empty())
			out.pop_back();
		return out;
	}
}

export namespace Boring32::Registry
{
	template<Win32::HKEY TParentKey, Strings::FixedStringW TSubKey, Strings::FixedStringW TValueName, ValueTypes TValueType>
		//requires CheckInvocable<TDefaultValue, TValueType> // not really required due to the static_asserts below
	class RegistryValue
	{
		static constexpr const wchar_t* SubKey = TSubKey;
		static constexpr const wchar_t* ValueName = TValueName;
		
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

			template<bool ThrowOnError = true, auto TDefaultValue = [] {}>
			static auto Read() noexcept(not ThrowOnError)
			{
				using ReturnDefaultType = std::invoke_result_t<decltype(TDefaultValue)>;

				if constexpr (TValueType == ValueTypes::DWord)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, Win32::DWORD>, "Return type from default lambda should be DWORD or convertible to DWORD.");
					return ReadValue<TValueType, ThrowOnError, TDefaultValue>(TParentKey, SubKey, ValueName);
				}
				else if constexpr (TValueType == ValueTypes::QWord)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, uint64_t>, "Return type from default lambda should be wstring or convertible to uint64_t.");
					return ReadValue<TValueType, ThrowOnError, TDefaultValue>(TParentKey, SubKey, ValueName);
				}
				else if constexpr (TValueType == ValueTypes::String)
				{
					static_assert(ThrowOnError or std::is_convertible_v<ReturnDefaultType, std::wstring>, "Return type from default lambda should be wstring or convertible to wstring.");
					return ReadValue<ThrowOnError, TDefaultValue>(TParentKey, SubKey, ValueName);
				}
				else
				{
					using T = RegistryValue<TParentKey, TSubKey, TValueName, TValueType>;
					static_assert(Concepts::AlwaysFalse<T>, "Unhandled value");
				}
			}

			static void Set(const auto& value)
			{
				using TValue = decltype(value);
				if constexpr (TValueType == ValueTypes::DWord)
				{
					static_assert(std::convertible_to<TValue, Win32::DWORD>, "Value should be DWORD or convertible to DWORD.");
				}
				else if constexpr (TValueType == ValueTypes::String)
				{
					static_assert(std::convertible_to<TValue, std::wstring>, "Value should be wstring or convertible to wstring.");
				}
				else if constexpr (TValueType == ValueTypes::QWord)
				{
					static_assert(std::convertible_to<TValue, uint64_t>, "Value should be uint64_t or convertible to uint64_t.");
				}
				else
				{
					using T = RegistryValue<TParentKey, TSubKey, TValueName, TValueType>;
					static_assert(Concepts::AlwaysFalse<T>, "Unhandled value");
				}

				SetValue<TValueType>(TParentKey, SubKey, ValueName, value);
			}

			template<bool Throw = true>
			void Delete()
			{
				Win32::HKEY key = nullptr;
				const Win32::LSTATUS status = Win32::RegOpenKeyExW(
					TParentKey,
					SubKey,
					0,
					Win32::_KEY_SET_VALUE,
					&key
				);
				if constexpr (Throw)
					if (status != Win32::ErrorCodes::Success)
						throw Error::Win32Error("Failed to open registry key", status);

				status = RegDeleteValueW(
					key,
					ValueName
				);
				Win32::RegCloseKey(key);
				if constexpr (Throw)
					if (status != Win32::ErrorCodes::Success)
						throw Error::Win32Error("Failed to delete registry value", status);
			}
	};

	template<Win32::HKEY Parent>
	struct SuperKeyName
	{
		std::wstring_view Name =
			[]() constexpr
			{
				if constexpr (Parent == Win32::_HKEY_CLASSES_ROOT)
					return L"HKEY_CLASSES_ROOT";
				else if constexpr (Parent == Win32::_HKEY_CURRENT_CONFIG)
					return L"HKEY_CURRENT_CONFIG";
				else if constexpr (Parent == Win32::_HKEY_CURRENT_USER)
					return L"HKEY_CURRENT_USER";
				else if constexpr (Parent == Win32::_HKEY_LOCAL_MACHINE)
					return L"HKEY_LOCAL_MACHINE";
				else if constexpr (Parent == Win32::_HKEY_USERS)
					return L"HKEY_USERS";
				else
					return L"";
			}();
	};

	// Test objects
	using M = RegistryValue<Win32::_HKEY_LOCAL_MACHINE, L"A", L"A", ValueTypes::DWord>;
	using N = RegistryValue<Win32::_HKEY_LOCAL_MACHINE, L"A", L"A", ValueTypes::QWord>;
	using O = RegistryValue<Win32::_HKEY_LOCAL_MACHINE, L"A", L"A", ValueTypes::String>;
	void XX()
	{
	//	//constexpr SuperKeyName<Win32::_HKEY_USERS> ll;
	//	//P<SuperKeyName<Win32::_HKEY_USERS>> p;

		M::Read<false, [] { return 1; }>();
		N::Read<false, [] { return 1; }>();
	//	O::Read<false, [] { return L""; }>();
	//	M::Set(1ull);
	//	O::Set(L"a");
	}
}
export module boring32:registry_registryvalue;
import boring32.shared;
import boring32.error;
import boring32.strings;
import boring32.concepts;

namespace Boring32::Registry
{
	using ValueTypes = Win32::Winreg::ValueTypes;

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
	void SetValue(Win32::Winreg::HKEY parentKey, const wchar_t* subKey, const wchar_t* valueName, const TValue& value)
	{
		Win32::Winreg::HKEY key = nullptr;
		Win32::LSTATUS status = Win32::Winreg::RegOpenKeyExW(
			parentKey,
			subKey,
			0,
			Win32::Winreg::_KEY_SET_VALUE,
			&key
		);
		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("Failed to open registry key", status);

		if constexpr (TValueType == ValueTypes::DWord or TValueType == ValueTypes::QWord)
		{
			status = Win32::Winreg::RegSetValueExW(
				key,
				valueName,
				0,
				Win32::DWORD(TValueType),
				reinterpret_cast<Win32::BYTE*>(const_cast<TValue*>(&value)),
				sizeof(value)
			);
		}
		else if constexpr (TValueType == ValueTypes::String)
		{
			static_assert(std::convertible_to<TValue, std::wstring_view>);
			std::wstring_view str{ value };
			status = Win32::Winreg::RegSetValueExW(
				key,
				valueName,
				0,
				Win32::DWORD(TValueType),
				reinterpret_cast<Win32::BYTE*>(const_cast<wchar_t*>(str.data())),
				static_cast<Win32::DWORD>((str.size() + 1) * sizeof(wchar_t))
			);
		}
		else
		{
			static_assert(Concepts::AlwaysFalse<TValue>);
		}
		Win32::Winreg::RegCloseKey(key);

		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("RegSetValueExW() failed", status);
	}

	template<ValueTypes TValueType>
	struct ValueTraits : std::false_type {};

	template<>
	struct ValueTraits<ValueTypes::DWord>
	{
		using ReturnType = Win32::DWORD;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::DWord);
		static constexpr size_t Size = sizeof(ReturnType);
	};
	template<>
	struct ValueTraits<ValueTypes::QWord>
	{
		using ReturnType = uint64_t;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::QWord);
		static constexpr size_t Size = sizeof(ReturnType);
	};
	template<>
	struct ValueTraits<ValueTypes::String>
	{
		using ReturnType = std::wstring;
		static constexpr Win32::DWORD RawType = Win32::DWORD(ValueTypes::String);
	};

	template<ValueTypes TValueType, bool ThrowOnError = true, Concepts::NullPtrOrInvocable auto DefaultValue = nullptr, typename TraitType = ValueTraits<TValueType>>
	TraitType::ReturnType ReadValue(Win32::Winreg::HKEY ParentKey, const wchar_t* SubKey, const wchar_t* ValueName)
	{
		if constexpr (TValueType == ValueTypes::DWord or TValueType == ValueTypes::QWord)
		{
			using ReturnType = typename TraitType::ReturnType;
			ReturnType out{};

			Win32::DWORD sizeInBytes = sizeof(out);
			Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
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
					return static_cast<ReturnType>(DefaultValue());
				else
					return {};
			}
			return out;
		}
		else if constexpr (TValueType == ValueTypes::String)
		{
			Win32::DWORD sizeInBytes = 0;
			// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
			Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
				ParentKey,
				SubKey,
				ValueName,
				Win32::Winreg::_RRF_RT_REG_SZ,
				nullptr,
				nullptr,
				&sizeInBytes
			);
			if (status != Win32::ErrorCodes::Success)
			{
				if constexpr (ThrowOnError)
					throw Error::Win32Error("RegGetValueW() failed", status);
				else if (std::invocable<decltype(DefaultValue)>)
					return { DefaultValue() };
				else
					return {};
			}

			std::wstring out(sizeInBytes / sizeof(wchar_t), '\0');
			status = Win32::Winreg::RegGetValueW(
				ParentKey,
				SubKey,
				ValueName,
				Win32::Winreg::_RRF_RT_REG_SZ,
				nullptr,
				&out[0],
				&sizeInBytes
			);
			if (status != Win32::ErrorCodes::Success)
			{
				if constexpr (ThrowOnError)
					throw Error::Win32Error("RegGetValueW() failed", status);
				else if (std::invocable<decltype(DefaultValue)>)
					return { DefaultValue() };
				else
					return {};
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
	Win32::DWORD ReadDWord(
		Win32::Winreg::HKEY ParentKey,
		const wchar_t* SubKey, 
		const wchar_t* ValueName
	) noexcept(not ThrowOnError)
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
		Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::Winreg::_RRF_RT_REG_DWORD,
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

	template<bool ThrowOnError = true, Concepts::NullPtrOrInvocable auto DefaultValue = nullptr>
	uint64_t ReadQWord(
		Win32::Winreg::HKEY ParentKey,
		const wchar_t* SubKey, 
		const wchar_t* ValueName
	) noexcept(not ThrowOnError)
	{
		static_assert(
			ThrowOnError
			or (
				not std::is_null_pointer_v<decltype(DefaultValue)>
				and std::is_invocable_v<decltype(DefaultValue)>
				and std::is_convertible_v<std::invoke_result_t<decltype(DefaultValue)>, DWORD>
			),
			"Either ThrowOnError must be true or DefaultValue must be an invocable returning a uint64_t"
		);

		uint64_t out;
		Win32::DWORD sizeInBytes = sizeof(out);
		Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::Winreg::_RRF_RT_REG_QWORD,
			nullptr,
			&out,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else if constexpr (std::is_invocable_v<decltype(DefaultValue)>)
				return { DefaultValue() };
			else
				return {};
		}
		return out;
	}

	template<bool ThrowOnError = true, Concepts::NullPtrOrInvocable auto DefaultValue = nullptr>
	std::wstring ReadString(
		Win32::Winreg::HKEY ParentKey,
		const wchar_t* SubKey, 
		const wchar_t* ValueName
	) noexcept(not ThrowOnError)
	{
		static_assert(
			ThrowOnError
			or (
				not std::is_null_pointer_v<decltype(DefaultValue)>
				and std::is_invocable_v<decltype(DefaultValue)>
				//and std::is_convertible_v<std::invoke_result_t<decltype(DefaultValue)>, DWORD>
			),
			"Either ThrowOnError must be true or DefaultValue must be an invocable returning a wstring"
		);

		Win32::DWORD sizeInBytes = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-reggetvaluew
		Win32::LSTATUS status = Win32::Winreg::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::Winreg::_RRF_RT_REG_SZ,
			nullptr,
			nullptr,
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else if constexpr (std::is_invocable_v<decltype(DefaultValue)>)
				return { DefaultValue() };
			else
				return {};
		}

		std::wstring out(sizeInBytes / sizeof(wchar_t), '\0');
		status = Win32::Winreg::RegGetValueW(
			ParentKey,
			SubKey,
			ValueName,
			Win32::Winreg::_RRF_RT_REG_SZ,
			nullptr,
			&out[0],
			&sizeInBytes
		);
		if (status != Win32::ErrorCodes::Success)
		{
			if constexpr (ThrowOnError)
				throw Error::Win32Error("RegGetValueW() failed", status);
			else if constexpr (std::is_invocable_v<decltype(DefaultValue)>)
				return { DefaultValue() };
			else
				return {};
		}

		out.resize(sizeInBytes / sizeof(wchar_t));
		// Exclude terminating null
		if (!out.empty())
			out.pop_back();
		return out;
	}

	template<bool ThrowOnError = true>
	void DeleteValue(
		Win32::Winreg::HKEY ParentKey,
		const wchar_t* SubKey,
		const wchar_t* ValueName
	)
	{
		Win32::Winreg::HKEY key = nullptr;
		Win32::LSTATUS status = Win32::Winreg::RegOpenKeyExW(
			ParentKey,
			SubKey,
			0,
			Win32::Winreg::_KEY_SET_VALUE,
			&key
		);
		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("Failed to open registry key", status);
			else
				return;

		status = Win32::Winreg::RegDeleteValueW(
			key,
			ValueName
		);
		Win32::Winreg::RegCloseKey(key);
		if constexpr (ThrowOnError)
			if (status != Win32::ErrorCodes::Success)
				throw Error::Win32Error("Failed to delete registry value", status);
	}
}

export namespace Boring32::Registry
{
	template<Win32::Winreg::HKEY Parent>
	struct SuperKeyName
	{
		static constexpr std::wstring_view Name =
			[]() constexpr
			{
				if constexpr (Parent == Win32::Winreg::_HKEY_CLASSES_ROOT)
					return L"HKEY_CLASSES_ROOT";
				else if constexpr (Parent == Win32::Winreg::_HKEY_CURRENT_CONFIG)
					return L"HKEY_CURRENT_CONFIG";
				else if constexpr (Parent == Win32::Winreg::_HKEY_CURRENT_USER)
					return L"HKEY_CURRENT_USER";
				else if constexpr (Parent == Win32::Winreg::_HKEY_LOCAL_MACHINE)
					return L"HKEY_LOCAL_MACHINE";
				else if constexpr (Parent == Win32::Winreg::_HKEY_USERS)
					return L"HKEY_USERS";
				else
					return L"Unknown";
			}();
	};

	struct HKEYDeleter final
	{
		void operator()(Win32::Winreg::HKEY ptr)
		{
			Win32::Winreg::RegCloseKey(ptr);
		}
	};
	using HKEYUniquePtr = std::unique_ptr<std::remove_pointer<Win32::Winreg::HKEY>::type, HKEYDeleter>;

	template<Win32::Winreg::HKEY TParentKey, Strings::FixedString TSubKey, bool DefaultThrowOnError = true>
	struct RegistryKey
	{
		static constexpr const wchar_t* SubKey = TSubKey;

		template<bool ThrowOnError = DefaultThrowOnError>
		static HKEYUniquePtr Create()
		{
			DWORD Win32::disposition = 0;
			Win32::Winreg::HKEY result = nullptr;
			const Win32::LSTATUS status = Win32::Winreg::RegCreateKeyExW(
				TParentKey,
				TSubKey,
				0,
				nullptr,
				0,
				Win32::Winreg::_KEY_ALL_ACCESS,
				nullptr,
				&result,
				&disposition
			);
			if constexpr (ThrowOnError)
			{
				if (status != Win32::ErrorCodes::Success)
				{
					const auto lastError = Win32::GetLastError();
					throw Error::Win32Error("RegCreateKeyExW() failed", lastError);
				}
			}
			return HKEYUniquePtr(result);
		}

		template<bool ThrowOnError = DefaultThrowOnError>
		static void Delete()
		{

		}
	};

	template<
		Win32::Winreg::HKEY TParentKey, 
		Strings::WideFixedString TSubKey, 
		Strings::WideFixedString TValueName,
		ValueTypes TValueType, 
		bool DefaultThrowOnError = true>
	class RegistryValue
	{
		static constexpr const wchar_t* SubKey = TSubKey;
		static constexpr const wchar_t* ValueName = TValueName;
		static constexpr std::wstring_view Name = SuperKeyName<TParentKey>::Name;

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

			template<bool ThrowOnError = DefaultThrowOnError, Concepts::NullPtrOrInvocable auto TDefaultValue = nullptr>
			static auto Read() noexcept(not ThrowOnError)
			{
				return ReadValue<TValueType, ThrowOnError, TDefaultValue>(TParentKey, SubKey, ValueName);
			}

			template<bool ThrowOnError = DefaultThrowOnError>
			static void Set(const auto& value) noexcept(not ThrowOnError)
			{
				SetValue<TValueType, ThrowOnError>(TParentKey, SubKey, ValueName, value);
			}

			template<bool ThrowOnError = DefaultThrowOnError>
			static void Delete() noexcept(not ThrowOnError)
			{
				DeleteValue<ThrowOnError>(TParentKey, SubKey, ValueName);
			}
	};
}
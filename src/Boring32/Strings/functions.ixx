export module boring32:strings.functions;
import std;
import std.compat;
import :win32;
import :error;
import :concepts;

export namespace Boring32::Strings
{
	[[nodiscard]]
	auto ConvertString(std::wstring_view wstr) -> std::string
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		auto sizeInBytes = 
			Win32::WideCharToMultiByte(
				Win32::CpUtf8,									// CodePage
				Win32::WcNoBestFitChars,						// dwFlags 
				wstr.data(),									// lpWideCharStr
				static_cast<int>(wstr.size()),					// cchWideChar 
				nullptr,										// lpMultiByteStr
				0,												// cbMultiByte
				nullptr,										// lpDefaultChar
				nullptr											// lpUsedDefaultChar
			);
		if (sizeInBytes == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "WideCharToMultiByte() [1] failed" };

		auto strTo = std::string(sizeInBytes / sizeof(char), '\0');
		auto status = 
			WideCharToMultiByte(
				Win32::CpUtf8,									// CodePage
				Win32::WcNoBestFitChars,						// dwFlags 
				wstr.data(),									// lpWideCharStr
				static_cast<int>(wstr.size()),					// cchWideChar 
				strTo.data(),									// lpMultiByteStr
				static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
				nullptr,										// lpDefaultChar
				nullptr											// lpUsedDefaultChar
			);
		if (status == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "WideCharToMultiByte() [2] failed" };

		return strTo;
	}

	[[nodiscard]]
	auto ConvertString(std::string_view str) -> std::wstring
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		auto sizeInCharacters = 
			Win32::MultiByteToWideChar(
				Win32::CpUtf8,									// CodePage
				0,											// dwFlags
				str.data(),									// lpMultiByteStr
				static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
				nullptr,									// lpWideCharStr
				0											// cchWideChar
			);
		if (sizeInCharacters == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() [1] failed" };

		auto wstrTo = std::wstring(sizeInCharacters, '\0');
		auto status = 
			Win32::MultiByteToWideChar(
				Win32::CpUtf8,									// CodePage
				0,											// dwFlags
				str.data(),									// lpMultiByteStr
				static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
				wstrTo.data(),									// lpWideCharStr
				static_cast<int>(wstrTo.size())				// cchWideChar
			);
		if (status == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "MultiByteToWideChar() [2] failed" };

		return wstrTo;
	}

	template<Concepts::WideOrNarrowString TString>
	[[nodiscard]]
	decltype(auto) To(Concepts::AnyString auto&& from)
	{
		if constexpr (Concepts::OneOf<decltype(from), TString&, const TString&>)
			return from;
		else if constexpr (std::is_constructible_v<TString, decltype(from)>)
			return TString{ std::forward<decltype(from)>(from) };
		else
			return ConvertString(from);
	}

	/* old implementation
	template<Concepts::WideOrNarrowString TString>
	auto To(const Concepts::AnyString auto& from)
	{

		if constexpr (std::same_as<TString, std::string>)
		{
			if constexpr (std::same_as<decltype(from), std::string&&>)
			{
				return std::string{ std::move(from) };
			}
			else if constexpr (std::same_as<T, std::string>)
			{
				return from;
			}
			else if constexpr (std::convertible_to<T, std::string_view>)
			{
				return std::string{ from };
			}
			else
			{
				return ConvertString(from);
			}
		}
		else
		{
			if constexpr (std::same_as<decltype(from), std::wstring&&>)
			{
				return std::wstring{ std::move(from) };
			}
			else if constexpr (std::same_as<T, std::wstring>) // covers wstring& and const wstring&.
			{
				return from;
			}
			else if constexpr (std::convertible_to<T, std::wstring_view>)
			{
				return std::wstring{ from };
			}
			else
			{
				return ConvertString(from);
			}
		}
	}
	*/

	template<Concepts::WideOrNarrowString TString>
	struct AutoString
	{
		using StringType = TString;
		using ViewType = std::conditional_t<std::same_as<TString, std::string>, std::string_view, std::wstring_view>;

		AutoString() = default;
		AutoString(Concepts::AnyString auto&& from)
			: Value{ To<TString>(std::forward<decltype(from)>(from)) } {}

		// operators
		explicit operator const TString& () const { return Value; }
		explicit operator TString()	const { return Value; }
		explicit operator ViewType() const { return { Value }; }
		auto operator==(Concepts::AnyString auto&& other) const -> bool
		{
			if constexpr (std::same_as<StringType, std::remove_cvref_t<decltype(other)>>)
				return Value == other;
			else
				return Value == To<TString>(std::forward<decltype(other)>(other));
		}
		void operator+=(Concepts::AnyString auto&& other)
		{
			Value += To<std::string>(std::forward<decltype(other)>(other));
		}
		template<typename T>
		auto operator==(const AutoString<T>& other) const -> bool
		{
			if constexpr (std::same_as<StringType, T>)
				return Value == other.Value;
			else
				return Value == To<TString>(other.Value);
		}

		// useful string functions
		constexpr auto begin() const noexcept { return Value.begin(); }
		constexpr auto end() const noexcept { return Value.end(); }
		constexpr auto c_str() const noexcept { return Value.c_str(); }
		constexpr auto data() const noexcept { return Value.data(); }

		TString Value{};
		/*friend bool operator==(const AutoString<std::string>& lhs, const AutoString<std::string>& rhs);
		friend bool operator==(const AutoString<std::string>& lhs, const AutoString<std::wstring>& rhs);
		friend bool operator==(const AutoString<std::wstring>& lhs, const AutoString<std::wstring>& rhs);*/
	};

	using AutoAnsi = AutoString<std::string>;
	using AutoWide = AutoString<std::wstring>;

	// trim from start (in place)
	void LeftTrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return not std::isspace(ch); }));
	}

	// trim from end (in place)
	void RightTrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
	}

	// trim from both ends (in place)
	void Trim(std::string& s)
	{
		LeftTrim(s);
		RightTrim(s);
	}

	// trim from start (in place)
	void LeftTrim(std::wstring& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
	}

	// trim from end (in place)
	void RightTrim(std::wstring& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {return !std::isspace(ch);}).base(), s.end());
	}

	// trim from both ends (in place)
	void Trim(std::wstring& s)
	{
		LeftTrim(s);
		RightTrim(s);
	}

	[[nodiscard]]
	auto TokeniseString(const std::string& stringToTokenise, const std::string& delimiter) -> std::vector<std::string>
	{
		auto position = size_t{};
		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
			return { stringToTokenise };

		auto results = std::vector<std::string>{};
		auto intermediateString = std::string{ stringToTokenise };
		while ((position = intermediateString.find(delimiter)) != std::string::npos)
		{
			// split and add to the results
			auto split = std::string{ intermediateString.substr(0, position) };
			results.push_back(split);

			// move up our position
			position += delimiter.length();
			intermediateString = intermediateString.substr(position);

			// On the last iteration, enter the remainder
			if (intermediateString.find(delimiter) == std::string::npos)
				results.push_back(intermediateString);
		}

		return results;
	}

	[[nodiscard]]
	auto TokeniseString(const std::wstring& stringToTokenise, const std::wstring& delimiter) -> std::vector<std::wstring>
	{
		auto position = size_t{};
		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
			return { stringToTokenise };

		auto results = std::vector<std::wstring>{};
		auto intermediateString = std::wstring{ stringToTokenise };
		while ((position = intermediateString.find(delimiter)) != std::string::npos)
		{
			// split and add to the results
			auto split = std::wstring{ intermediateString.substr(0, position) };
			results.push_back(split);

			// move up our position
			position += delimiter.length();
			intermediateString = intermediateString.substr(position);

			// On the last iteration, enter the remainder
			if (intermediateString.find(delimiter) == std::string::npos)
				results.push_back(intermediateString);
		}

		return results;
	}

	// Adapted from https://stackoverflow.com/a/29752943/7448661
	[[nodiscard]]
	auto Replace(std::wstring source, const std::wstring& from, const std::wstring& to) -> std::wstring
	{
		auto newString = std::wstring{};
		newString.reserve(source.length());

		auto lastPos = std::wstring::size_type{};
		auto findPos = std::wstring::size_type{};
		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}
		newString += source.substr(lastPos);
		return newString;
	}

	[[nodiscard]]
	auto Erase(std::wstring source, const wchar_t what) -> std::wstring
	{
		source.erase(std::remove(source.begin(), source.end(), what), source.end());
		return source;
	}

	// Adapted from https://www.tutorialspoint.com/case-insensitive-string-comparison-in-cplusplus
	[[nodiscard]]
	auto DoCaseInsensitiveMatch(std::string_view str1, std::string_view str2) -> bool
	{
		auto w1 = ConvertString(str1);
		auto w2 = ConvertString(str2);
		return Win32::CompareStringOrdinal(
			w1.data(),
			static_cast<int>(w1.size()),
			w2.data(),
			static_cast<int>(w2.size()),
			true
		) == Win32::CStrComparison::Equal;
	}

	[[nodiscard]]
	auto DoCaseInsensitiveMatch(std::wstring_view str1, std::wstring_view str2) -> bool
	{
		return Win32::CompareStringOrdinal(
			str1.data(), 
			static_cast<int>(str1.size()), 
			str2.data(), 
			static_cast<int>(str2.size()), 
			true
		) == Win32::CStrComparison::Equal;
	}

	struct ArgInfo
	{
		std::vector<wchar_t> Argv;
		unsigned Argc = 0;;
	};

	[[nodiscard]]
	auto StringsToArgVector(const std::vector<std::wstring>& args) -> ArgInfo
	{
		auto info = ArgInfo{};
		for (const std::wstring& arg : args)
		{
			if (arg.empty())
				continue;
			info.Argv.insert(info.Argv.end(), arg.begin(), arg.end());
			info.Argv.push_back('\0');
			info.Argc++;
		}
		return info;
	}

	// Must be either nullptr, or a nothrowing invocable that accepts 
	// an std::exception and returns something convertible to either 
	// a string or wstring.
	template<typename T>
	concept IsExpectedInvocable =
		std::is_null_pointer_v<T>
		or std::is_nothrow_invocable_v<T, const std::exception&>
		and (
			requires(T t) { { t(std::exception{}) } -> std::convertible_to<std::string>; }
			or
			requires(T t) { { t(std::exception{}) } -> std::convertible_to<std::wstring>; }
		);

	template<IsExpectedInvocable auto TDefault = nullptr>
	[[nodiscard]]
	auto SafeVFormat(
		Concepts::IsStringType auto&& param,
		auto&&...args
	) noexcept -> decltype(std::vformat(param, std::make_format_args(args...)))
	try
	{
		if constexpr (TDefault)
		{
			static_assert(
				Concepts::IsString<decltype(param)> and Concepts::IsString<std::invoke_result_t<decltype(TDefault), std::exception>>
				or Concepts::IsWideString<decltype(param)> and Concepts::IsWideString<std::invoke_result_t<decltype(TDefault), const std::exception&>>,
				"Default function return type must match the type of string"
			);
		}
		return std::vformat(param, std::make_format_args(args...));
	}
	catch (const std::exception& ex)
	{
		if constexpr (TDefault)
			return TDefault(ex);
		else if constexpr (Concepts::IsString<decltype(param)>)
			return std::string{};
		else
			return std::wstring{};
	}

	template<typename...TArgs>
	[[nodiscard]]
	auto Format(std::format_string<TArgs...> fmt, TArgs&&...args) -> std::string
	{
		auto buffer = std::string{};
		std::format_to(
			std::back_inserter(buffer),
			fmt,
			std::forward<TArgs>(args)...);
		return buffer;
	}

	[[nodiscard]]
	auto ChangeCase(const std::wstring& source, bool upper) -> std::wstring
	{
		auto flag = upper
			? Win32::i18n::LcMap::UpperCase
			: Win32::i18n::LcMap::LowerCase;

		// https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringex
		// See also https://devblogs.microsoft.com/oldnewthing/20241007-00/?p=110345
		auto result = 
			Win32::i18n::LCMapStringEx(
				Win32::i18n::Locales::Invariant,
				flag,
				source.data(),
				static_cast<int>(source.size()),
				nullptr,
				0,
				nullptr,
				nullptr,
				0
			);
		if (result == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "LCMapStringEx() failed." };

		auto destination = std::wstring(result, '\0');
		result = 
			Win32::i18n::LCMapStringEx(
				Win32::i18n::Locales::Invariant,
				flag,
				source.data(),
				static_cast<int>(source.size()),
				destination.data(),
				static_cast<int>(destination.size()),
				nullptr,
				nullptr,
				0
			);
		if (result == 0)
			throw Error::Win32Error{ Win32::GetLastError(), "LCMapStringEx() failed." };

		return destination;
	}

	[[nodiscard]]
	auto ToUpper(const std::wstring& string) -> std::wstring { return ChangeCase(string, true); }
	[[nodiscard]]
	auto ToLower(const std::wstring& string) -> std::wstring { return ChangeCase(string, false); }
}

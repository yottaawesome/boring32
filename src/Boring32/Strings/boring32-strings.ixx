export module boring32:strings;
import std;
import std.compat;
import boring32.win32;
import :error;
import :concepts;
export import :strings_fixedstring;

export namespace Boring32::Strings
{
	std::string ConvertString(std::wstring_view wstr)
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		const int sizeInBytes = Win32::WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			nullptr,										// lpMultiByteStr
			0,												// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (sizeInBytes == 0)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("WideCharToMultiByte() [1] failed", lastError);
		}

		std::string strTo(sizeInBytes / sizeof(char), '\0');
		const int status = WideCharToMultiByte(
			Win32::CpUtf8,										// CodePage
			Win32::WcNoBestFitChars,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			&strTo[0],										// lpMultiByteStr
			static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (status == 0)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("WideCharToMultiByte() [2] failed", lastError);
		}

		return strTo;
	}

	std::wstring ConvertString(std::string_view str)
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		const int sizeInCharacters = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
			nullptr,									// lpWideCharStr
			0											// cchWideChar
		);
		if (sizeInCharacters == 0)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("MultiByteToWideChar() [1] failed", lastError);
		}

		std::wstring wstrTo(sizeInCharacters, '\0');
		const int status = Win32::MultiByteToWideChar(
			Win32::CpUtf8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
			&wstrTo[0],									// lpWideCharStr
			static_cast<int>(wstrTo.size())				// cchWideChar
		);
		if (status == 0)
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("MultiByteToWideChar() [2] failed", lastError);
		}

		return wstrTo;
	}

	template<Concepts::WideOrNarrowString TString>
	decltype(auto) To(Concepts::AnyString auto&& from)
	{
		if constexpr (Concepts::OneOf<decltype(from), TString&, const TString&>)
		{
			return from;
		}
		else if constexpr (std::is_constructible_v<TString, decltype(from)>)
		{
			return TString{ std::forward<decltype(from)>(from) };
		}
		else
		{
			return ConvertString(from);
		}
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
			: Value{ To<TString>(std::forward<decltype(from)>(from)) } { }
		
		// operators
		explicit operator const TString&() const { return Value; }
		explicit operator TString()	const { return Value; }
		explicit operator ViewType() const { return { Value }; }
		bool operator==(Concepts::AnyString auto&& other) const
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
		bool operator==(const AutoString<T>& other) const
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
		s.erase(
			s.begin(),
			std::find_if(
				s.begin(),
				s.end(),
				[](int ch) { return !std::isspace(ch); }
			)
		);
	}

	// trim from end (in place)
	void RightTrim(std::string& s)
	{
		s.erase(
			std::find_if(
				s.rbegin(), 
				s.rend(), 
				[](int ch) { return !std::isspace(ch); }
			).base(), 
			s.end()
		);
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
		s.erase(
			s.begin(),
			std::find_if(
				s.begin(),
				s.end(),
				[](int ch) { return !std::isspace(ch); }
			)
		);
	}

	// trim from end (in place)
	void RightTrim(std::wstring& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
			}).base(), s.end());
	}

	// trim from both ends (in place)
	void Trim(std::wstring& s)
	{
		LeftTrim(s);
		RightTrim(s);
	}

	std::vector<std::string> TokeniseString(
		const std::string& stringToTokenise,
		const std::string& delimiter
	)
	{
		size_t position = 0;
		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
			return { stringToTokenise };

		std::vector<std::string> results;
		std::string intermediateString = stringToTokenise;
		while ((position = intermediateString.find(delimiter)) != std::string::npos)
		{
			// split and add to the results
			std::string split = intermediateString.substr(0, position);
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

	std::vector<std::wstring> TokeniseString(
		const std::wstring& stringToTokenise,
		const std::wstring& delimiter
	)
	{
		size_t position = 0;
		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
			return { stringToTokenise };

		std::vector<std::wstring> results;
		std::wstring intermediateString = stringToTokenise;
		while ((position = intermediateString.find(delimiter)) != std::string::npos)
		{
			// split and add to the results
			std::wstring split = intermediateString.substr(0, position);
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
	std::wstring Replace(
		std::wstring source,
		const std::wstring& from,
		const std::wstring& to
	)
	{
		std::wstring newString;
		newString.reserve(source.length());

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type findPos;

		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		newString += source.substr(lastPos);

		return newString;
	}

	std::wstring Erase(std::wstring source, const wchar_t what)
	{
		source.erase(
			std::remove(source.begin(), source.end(), what), 
			source.end()
		);
		return source;
	}

	// Adapted from https://www.tutorialspoint.com/case-insensitive-string-comparison-in-cplusplus
	bool DoCaseInsensitiveMatch(std::string str1, std::string str2)
	{
		// Weirdly, transform() fails to compile if passing to lower() directly to transform().
		constexpr auto lower = [](char c) { return std::tolower(c); };
		//convert s1 and s2 into lower case strings
		std::transform(str1.begin(), str1.end(), str1.begin(), lower);
		std::transform(str2.begin(), str2.end(), str2.begin(), lower);
		return str1 == str2;
	}

	bool DoCaseInsensitiveMatch(std::wstring str1, std::wstring str2)
	{
		//convert s1 and s2 into lower case strings
		constexpr auto lower = [](wchar_t c) { return std::tolower(c); };
		std::transform(str1.begin(), str1.end(), str1.begin(), lower);
		std::transform(str2.begin(), str2.end(), str2.begin(), lower);
		return str1 == str2;
	}

	struct ArgInfo
	{
		std::vector<wchar_t> Argv;
		unsigned Argc = 0;;
	};

	ArgInfo StringsToArgVector(const std::vector<std::wstring>& args)
	{
		ArgInfo info{};
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
			requires(T t) { {t(std::exception{})} -> std::convertible_to<std::string>; }
			or 
			requires(T t) { {t(std::exception{})} -> std::convertible_to<std::wstring>; }
		);

	template<IsExpectedInvocable auto TDefault = nullptr>
	auto SafeVFormat(
		Concepts::IsStringType auto&& param,
		auto&&...args
	) noexcept try
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
	std::string Format(std::format_string<TArgs...> fmt, TArgs&&...args)
	{
		std::string buffer;
		std::format_to(
			std::back_inserter(buffer),
			fmt,
			std::forward<TArgs>(args)...);
		return buffer;
	}

	[[nodiscard]]
	std::wstring ChangeCase(const std::wstring& source, const bool upper)
	{
		const int flag = upper 
			? Win32::i18n::LcMap::UpperCase 
			: Win32::i18n::LcMap::LowerCase;

		// https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringex
		// See also https://devblogs.microsoft.com/oldnewthing/20241007-00/?p=110345
		int result = Win32::i18n::LCMapStringEx(
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
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("LCMapStringEx() failed.", lastError);
		}

		std::wstring destination(result, '\0');
		result = Win32::i18n::LCMapStringEx(
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
		{
			const auto lastError = Win32::GetLastError();
			throw Error::Win32Error("LCMapStringEx() failed.", lastError);
		}

		return destination;
	}

	[[nodiscard]]
	std::wstring ToUpper(const std::wstring& string) { return ChangeCase(string, true); }
	[[nodiscard]]
	std::wstring ToLower(const std::wstring& string) { return ChangeCase(string, false); }
}
export module boring32:strings_fixedstring;
import boring32.shared;

export namespace Boring32::Strings
{
	// See https://dev.to/sgf4/strings-as-template-parameters-c20-4joh
	template<typename T>
	concept ValidCharType = std::same_as<T, char> or std::same_as<T, wchar_t>;

	template<typename T>
	concept ValidViewType = std::same_as<T, std::string_view> or std::same_as<T, std::wstring_view>;

	template<typename T>
	concept ValidStringType = std::same_as<T, std::string> or std::same_as<T, std::wstring>;

	template <ValidCharType TChar, ValidViewType TView, ValidStringType TString, size_t N>
	struct FixedString
	{
		using CharType = TChar;
		using ViewType = TView;
		using StringType = TString;

		TChar buf[N]{};

		consteval FixedString(const TChar(&arg)[N]) noexcept
		{
			std::copy_n(arg, N, buf);
		}

		//constexpr FixedString(const wchar_t* arg) noexcept
		//{
		//	std::copy_n(arg, N, buf);
		//	//for (unsigned i = 0; i < N; i++)
		//		//buf[i] = arg[i];
		//}

		// There's a consteval bug in the compiler.
		// See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
		constexpr operator const TChar* () const noexcept
		{
			return buf;
		}

		consteval TView ToView() const noexcept
		{
			return { buf };
		}

		consteval operator TView() const noexcept
		{
			return { buf };
		}

		constexpr operator TString() const noexcept
		{
			return { buf };
		}

		constexpr TString ToString() const noexcept
		{
			return { buf };
		}

		consteval bool operator==(const FixedString<TChar, TView, TString, N> str) const
		{
			return std::equal(str.buf, str.buf + N, buf);
		}

		template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t N2>
		consteval bool operator==(const FixedString<TChar, TView, TString, N2> s) const
		{
			return false;
		}

		template<std::size_t N2>
		consteval FixedString<TChar, TView, TString, N + N2 - 1> operator+(const FixedString<TChar, TView, TString, N2> str) const
		{
			TChar newchar[N + N2 - 1]{};
			std::copy_n(buf, N - 1, newchar);
			std::copy_n(str.buf, N2, newchar + N - 1);
			return newchar;
		}
	};
	template<size_t N>
	FixedString(char const (&)[N]) -> FixedString<char, std::string_view, std::string, N>;
	template<size_t N>
	FixedString(wchar_t const (&)[N]) -> FixedString<wchar_t, std::wstring_view, std::wstring, N>;

	template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t s1, std::size_t s2>
	consteval auto operator+(FixedString<TChar, TView, TString, s1> fs, const TChar(&str)[s2])
	{
		return fs + FixedString<TChar, TView, TString, s2>(str);
	}

	template<ValidCharType TChar, ValidViewType TView, ValidStringType TString, std::size_t s1, std::size_t s2>
	consteval auto operator+(const TChar(&str)[s2], FixedString<TChar, TView, TString, s1> fs)
	{
		return FixedString<s2>(str) + fs;
	}

	template<size_t N>
	using WideFixedString = FixedString<wchar_t, std::wstring_view, std::wstring, N>;
	template<size_t N>
	using NarrowFixedString = FixedString<char, std::string_view, std::string, N>;
}
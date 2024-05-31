export module boring32.strings:fixedstring;
import boring32.shared;

export namespace Boring32::Strings
{
	template <typename TChar, typename TView, typename TString, size_t N>
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
	};
	template<size_t N>
	FixedString(char const (&)[N]) -> FixedString<char, std::string_view, std::string, N>;
	template<size_t N>
	FixedString(wchar_t const (&)[N]) -> FixedString<wchar_t, std::wstring_view, std::wstring, N>;
}
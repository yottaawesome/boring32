export module boring32.strings:fixedstring;
import boring32.shared;

export namespace Boring32::Strings
{
	template <size_t N>
	struct FixedStringW
	{
		// There's a consteval bug in the compiler.
		// See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
		wchar_t buf[N]{};
		
		constexpr FixedStringW(const wchar_t(&arg)[N]) noexcept
		{
			std::copy_n(arg, N, buf);
		}

		//constexpr FixedString(const wchar_t* arg) noexcept
		//{
		//	std::copy_n(arg, N, buf);
		//	//for (unsigned i = 0; i < N; i++)
		//		//buf[i] = arg[i];
		//}

		constexpr operator const wchar_t* () const noexcept
		{
			return buf;
		}

		constexpr operator std::wstring_view() const noexcept
		{
			return { buf };
		}

		operator std::wstring() const noexcept
		{
			return { buf };
		}

		std::wstring ToString() const noexcept
		{
			return { buf };
		}

		std::wstring_view ToView() const noexcept
		{
			return { buf };
		}
	};
	template<size_t N>
	FixedStringW(wchar_t const (&)[N]) -> FixedStringW<N>;

	template <size_t N>
	struct FixedStringN
	{
		// There's a consteval bug in the compiler.
		// See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
		char buf[N]{};

		constexpr FixedStringN(const char(&arg)[N]) noexcept
		{
			std::copy_n(arg, N, buf);
		}

		constexpr operator const char*() const noexcept
		{
			return buf;
		}

		constexpr operator std::string_view() const noexcept
		{
			return { buf };
		}

		operator std::string() const noexcept
		{
			return { buf };
		}

		std::string ToString() const noexcept
		{
			return { buf };
		}

		std::string_view ToView() const noexcept
		{
			return { buf };
		}
	};
	template<size_t N>
	FixedStringN(char* const (&)[N]) -> FixedStringN<N>;
}
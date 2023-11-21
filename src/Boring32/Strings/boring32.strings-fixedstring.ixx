export module boring32.strings:fixedstring;
import std;
import std.compat;

export namespace Boring32::Strings
{
	template <size_t N>
	struct FixedString
	{
		// There's a consteval bug in the compiler.
		// See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
		wchar_t buf[N]{};
		
		constexpr FixedString(const wchar_t(&arg)[N]) noexcept
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
			return { buf, N };
		}

		operator std::wstring() const noexcept
		{
			return { buf, N };
		}

		std::wstring ToString() const noexcept
		{
			return { buf, N };
		}
	};
	template<size_t N>
	FixedString(wchar_t const (&)[N]) -> FixedString<N>;
}
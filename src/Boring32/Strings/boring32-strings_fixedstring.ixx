export module boring32:strings_fixedstring;
import std;

export namespace Boring32::Strings
{
	// See https://dev.to/sgf4/strings-as-template-parameters-c20-4joh
	template<typename T>
	concept ValidCharType = std::same_as<T, char> or std::same_as<T, wchar_t>;

	template <ValidCharType TChar, size_t N>
	struct FixedString
	{
		using CharType = TChar;

		TChar Buffer[N]{};

		constexpr FixedString(const TChar(&arg)[N]) noexcept
		{
			std::copy_n(arg, N, Buffer);
		}

		//constexpr FixedString(const wchar_t* arg) noexcept
		//{
		//	std::copy_n(arg, N, Buffer);
		//	//for (unsigned i = 0; i < N; i++)
		//		//Buffer[i] = arg[i];
		//}

		// There's a consteval bug in the compiler.
		// See https://developercommunity.visualstudio.com/t/consteval-function-unexpectedly-returns/10501040
		[[nodiscard]]
		constexpr operator const TChar*() const noexcept
		{
			return Buffer;
		}

		[[nodiscard]]
		constexpr std::basic_string_view<TChar, std::char_traits<TChar>> ToView() const noexcept
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr operator std::basic_string_view<TChar, std::char_traits<TChar>>() const noexcept
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr operator std::basic_string<TChar, std::char_traits<TChar>>() const noexcept
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr std::basic_string<TChar, std::char_traits<TChar>> ToString() const noexcept
		{
			return { Buffer };
		}

		template<size_t M>
		[[nodiscard]]
		constexpr bool operator==(const TChar(&str)[M]) const noexcept
		{
			return false;
		}

		[[nodiscard]]
		constexpr bool operator==(const TChar(&str)[N]) const noexcept
		{
			return std::equal(str, str + N, Buffer);
		}

		[[nodiscard]]
		constexpr bool operator==(const FixedString<TChar, N> str) const
		{
			return std::equal(str.Buffer, str.Buffer + N, Buffer);
		}

		[[nodiscard]]
		constexpr size_t Size() const noexcept { return N-1; }

		template<ValidCharType TChar, std::size_t N2>
		[[nodiscard]]
		constexpr bool operator==(const FixedString<TChar, N2> s) const
		{
			return false;
		}

		template<std::size_t N2>
		[[nodiscard]]
		constexpr FixedString<TChar, N + N2 - 1> operator+(const FixedString<TChar, N2> str) const
		{
			TChar newchar[N + N2 - 1]{};
			std::copy_n(Buffer, N - 1, newchar);
			std::copy_n(str.Buffer, N2, newchar + N - 1);
			return newchar;
		}

		struct Iterator
		{
			const char* Buffer = nullptr;
			int Position = 0;
			constexpr Iterator(int position, const char* buffer) : Position(position), Buffer(buffer) {}
			[[nodiscard]]
			constexpr char operator*() const noexcept { return Buffer[Position]; }
			constexpr Iterator& operator++() noexcept { Position++; return *this; }
			[[nodiscard]]
			constexpr bool operator!=(const Iterator& other) const noexcept { return Position != other.Position; }
		};

		[[nodiscard]]
		Iterator begin() const noexcept { return Iterator(0, Buffer); }
		[[nodiscard]]
		Iterator end() const noexcept { return Iterator(N - 1, Buffer); }
	};
	template<size_t N>
	FixedString(char const (&)[N]) -> FixedString<char, N>;
	template<size_t N>
	FixedString(wchar_t const (&)[N]) -> FixedString<wchar_t, N>;

	template<ValidCharType TChar, std::size_t s1, std::size_t s2>
	constexpr auto operator+(FixedString<TChar, s1> fs, const TChar(&str)[s2])
	{
		return fs + FixedString<TChar, s2>(str);
	}

	template<ValidCharType TChar, std::size_t s1, std::size_t s2>
	constexpr auto operator+(const TChar(&str)[s2], FixedString<TChar, s1> fs)
	{
		return FixedString<s2>(str) + fs;
	}

	template<size_t N>
	using FixedStringW = FixedString<wchar_t, N>;
	template<size_t N>
	using FixedStringA = FixedString<char, N>;
}
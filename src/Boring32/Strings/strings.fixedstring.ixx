export module boring32:strings.fixedstring;
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
		using String = std::basic_string<TChar>;
		using View = std::basic_string_view<TChar>;

		TChar Buffer[N]{};

		constexpr FixedString() noexcept = default;

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
		constexpr auto ToView() const noexcept -> View
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr operator View() const noexcept
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr operator String() const noexcept
		{
			return { Buffer };
		}

		[[nodiscard]]
		constexpr String ToString() const noexcept
		{
			return { Buffer };
		}

		template<size_t M>
		[[nodiscard]]
		constexpr auto operator==(const TChar(&str)[M]) const noexcept -> bool
		{
			return false;
		}

		[[nodiscard]]
		constexpr auto operator==(const TChar(&str)[N]) const noexcept -> bool
		{
			return std::equal(str, str + N, Buffer);
		}

		[[nodiscard]]
		constexpr auto operator==(const FixedString<TChar, N> str) const -> bool
		{
			return std::equal(str.Buffer, str.Buffer + N, Buffer);
		}

		[[nodiscard]]
		constexpr auto Size() const noexcept -> size_t { return N-1; }

		template<ValidCharType TChar, std::size_t N2>
		[[nodiscard]]
		constexpr auto operator==(const FixedString<TChar, N2> s) const -> bool
		{
			return false;
		}

		template<std::size_t N2>
		[[nodiscard]]
		constexpr auto operator+(const FixedString<TChar, N2> str) const -> FixedString<TChar, N + N2 - 1>
		{
			TChar newchar[N + N2 - 1]{};
			std::copy_n(Buffer, N - 1, newchar);
			std::copy_n(str.Buffer, N2, newchar + N - 1);
			return newchar;
		}

		struct Iterator
		{
			const CharType* Buffer = nullptr;
			int Position = 0;
			constexpr Iterator(int position, const char* buffer) noexcept
				: Position(position), Buffer(buffer) 
			{}
			constexpr auto operator++(this Iterator& self) noexcept -> Iterator& 
			{ 
				Position++; return self; 
			}
			[[nodiscard]]
			constexpr auto operator*(this const Iterator& self) noexcept -> CharType
			{ 
				return self.Buffer[Position]; 
			}
			[[nodiscard]]
			constexpr auto operator!=(this const Iterator& self, const Iterator& other) noexcept -> bool 
			{ 
				return self.Position != other.Position; 
			}
		};

		[[nodiscard]]
		auto begin() const noexcept -> Iterator { return Iterator(0, Buffer); }
		[[nodiscard]]
		auto end() const noexcept { return Iterator(N - 1, Buffer); }
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
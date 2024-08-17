export module boring32:strongtypes_functionalities;
import boring32.shared;
import :strongtypes_crtp;
import :strongtypes_impl;

export namespace Boring32::StrongTypes
{
	template <typename T>
	struct PreIncrementable : crtp<T, PreIncrementable>
	{
		constexpr T& operator++()
		{
			++this->underlying().get();
			return this->underlying();
		}
	};

	template <typename T>
	struct PostIncrementable : crtp<T, PostIncrementable>
	{
		constexpr T operator++(int)
		{
			return T(this->underlying().get()++);
		}
	};

	template <typename T>
	struct PreDecrementable : crtp<T, PreDecrementable>
	{
		constexpr T& operator--()
		{
			--this->underlying().get();
			return this->underlying();
		}
	};

	template <typename T>
	struct PostDecrementable : crtp<T, PostDecrementable>
	{
		constexpr T operator--(int)
		{
			return T(this->underlying().get()--);
		}
	};

	template <typename T>
	struct BinaryAddable : crtp<T, BinaryAddable>
	{
		[[nodiscard]] constexpr T operator+(T const& other) const
		{
			return T(this->underlying().get() + other.get());
		}
		constexpr T& operator+=(T const& other)
		{
			this->underlying().get() += other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct UnaryAddable : crtp<T, UnaryAddable>
	{
		[[nodiscard]] constexpr T operator+() const
		{
			return T(+this->underlying().get());
		}
	};

	template <typename T>
	struct __declspec(empty_bases)Addable
		: BinaryAddable<T>
		, UnaryAddable<T>
	{
		using BinaryAddable<T>::operator+;
		using UnaryAddable<T>::operator+;
	};

	template <typename T>
	struct BinarySubtractable : crtp<T, BinarySubtractable>
	{
		[[nodiscard]] constexpr T operator-(T const& other) const
		{
			return T(this->underlying().get() - other.get());
		}
		constexpr T& operator-=(T const& other)
		{
			this->underlying().get() -= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct UnarySubtractable : crtp<T, UnarySubtractable>
	{
		[[nodiscard]] constexpr T operator-() const
		{
			return T(-this->underlying().get());
		}
	};

	template <typename T>
	struct __declspec(empty_bases)Subtractable
		: BinarySubtractable<T>
		, UnarySubtractable<T>
	{
		using UnarySubtractable<T>::operator-;
		using BinarySubtractable<T>::operator-;
	};

	template <typename T>
	struct Multiplicable : crtp<T, Multiplicable>
	{
		[[nodiscard]] constexpr T operator*(T const& other) const
		{
			return T(this->underlying().get() * other.get());
		}
		constexpr T& operator*=(T const& other)
		{
			this->underlying().get() *= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct Divisible : crtp<T, Divisible>
	{
		[[nodiscard]] constexpr T operator/(T const& other) const
		{
			return T(this->underlying().get() / other.get());
		}
		constexpr T& operator/=(T const& other)
		{
			this->underlying().get() /= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct Modulable : crtp<T, Modulable>
	{
		[[nodiscard]] constexpr T operator%(T const& other) const
		{
			return T(this->underlying().get() % other.get());
		}
		constexpr T& operator%=(T const& other)
		{
			this->underlying().get() %= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct BitWiseInvertable : crtp<T, BitWiseInvertable>
	{
		[[nodiscard]] constexpr T operator~() const
		{
			return T(~this->underlying().get());
		}
	};

	template <typename T>
	struct BitWiseAndable : crtp<T, BitWiseAndable>
	{
		[[nodiscard]] constexpr T operator&(T const& other) const
		{
			return T(this->underlying().get() & other.get());
		}
		constexpr T& operator&=(T const& other)
		{
			this->underlying().get() &= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct BitWiseOrable : crtp<T, BitWiseOrable>
	{
		[[nodiscard]] constexpr T operator|(T const& other) const
		{
			return T(this->underlying().get() | other.get());
		}
		constexpr T& operator|=(T const& other)
		{
			this->underlying().get() |= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct BitWiseXorable : crtp<T, BitWiseXorable>
	{
		[[nodiscard]] constexpr T operator^(T const& other) const
		{
			return T(this->underlying().get() ^ other.get());
		}
		constexpr T& operator^=(T const& other)
		{
			this->underlying().get() ^= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct BitWiseLeftShiftable : crtp<T, BitWiseLeftShiftable>
	{
		[[nodiscard]] constexpr T operator<<(T const& other) const
		{
			return T(this->underlying().get() << other.get());
		}
		constexpr T& operator<<=(T const& other)
		{
			this->underlying().get() <<= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct BitWiseRightShiftable : crtp<T, BitWiseRightShiftable>
	{
		[[nodiscard]] constexpr T operator>>(T const& other) const
		{
			return T(this->underlying().get() >> other.get());
		}
		constexpr T& operator>>=(T const& other)
		{
			this->underlying().get() >>= other.get();
			return this->underlying();
		}
	};

	template <typename T>
	struct Comparable : crtp<T, Comparable>
	{
		[[nodiscard]] constexpr bool operator<(Comparable<T> const& other) const
		{
			return this->underlying().get() < other.underlying().get();
		}
		[[nodiscard]] constexpr bool operator>(Comparable<T> const& other) const
		{
			return other.underlying().get() < this->underlying().get();
		}
		[[nodiscard]] constexpr bool operator<=(Comparable<T> const& other) const
		{
			return !(other < *this);
		}
		[[nodiscard]] constexpr bool operator>=(Comparable<T> const& other) const
		{
			return !(*this < other);
		}
		[[nodiscard]] constexpr bool operator==(Comparable<T> const& other) const
		{
			return !(*this < other) && !(other < *this);
		}
		[[nodiscard]] constexpr bool operator!=(Comparable<T> const& other) const
		{
			return !(*this == other);
		}
	};

	template< typename T >
	struct Dereferencable;

	template< typename T, typename Parameter, template< typename > class ... Skills >
	struct Dereferencable<NamedType<T, Parameter, Skills...>> : crtp<NamedType<T, Parameter, Skills...>, Dereferencable>
	{
		[[nodiscard]] constexpr T& operator*()&
		{
			return this->underlying().get();
		}
		[[nodiscard]] constexpr std::remove_reference_t<T> const& operator*() const&
		{
			return this->underlying().get();
		}
	};

	template <typename Destination>
	struct ImplicitlyConvertibleTo
	{
		template <typename T>
		struct templ : crtp<T, templ>
		{
			[[nodiscard]] constexpr operator Destination() const
			{
				return this->underlying().get();
			}
		};
	};

	template <typename T>
	struct Printable : crtp<T, Printable>
	{
		static constexpr bool is_printable = true;

		void print(std::ostream& os) const
		{
			os << this->underlying().get();
		}
	};

	template <typename T, typename Parameter, template <typename> class... Skills>
	typename std::enable_if<NamedType<T, Parameter, Skills...>::is_printable, std::ostream&>::type
		operator<<(std::ostream& os, NamedType<T, Parameter, Skills...> const& object)
	{
		object.print(os);
		return os;
	}

	template <typename T>
	struct Hashable
	{
		static constexpr bool is_hashable = true;
	};

	template <typename NamedType_>
	struct FunctionCallable;

	template <typename T, typename Parameter, template <typename> class... Skills>
	struct FunctionCallable<NamedType<T, Parameter, Skills...>> : crtp<NamedType<T, Parameter, Skills...>, FunctionCallable>
	{
		[[nodiscard]] constexpr operator T const& () const
		{
			return this->underlying().get();
		}
		[[nodiscard]] constexpr operator T& ()
		{
			return this->underlying().get();
		}
	};

	template <typename NamedType_>
	struct MethodCallable;

	template <typename T, typename Parameter, template <typename> class... Skills>
	struct MethodCallable<NamedType<T, Parameter, Skills...>> : crtp<NamedType<T, Parameter, Skills...>, MethodCallable>
	{
		[[nodiscard]] constexpr std::remove_reference_t<T> const* operator->() const
		{
			return std::addressof(this->underlying().get());
		}
		[[nodiscard]] constexpr std::remove_reference_t<T>* operator->()
		{
			return std::addressof(this->underlying().get());
		}
	};

	template <typename NamedType_>
	struct __declspec(empty_bases)Callable
		: FunctionCallable<NamedType_>
		, MethodCallable<NamedType_>
	{
	};

	template <typename T>
	struct __declspec(empty_bases)Incrementable
		: PreIncrementable<T>
		, PostIncrementable<T>
	{
		using PostIncrementable<T>::operator++;
		using PreIncrementable<T>::operator++;
	};

	template <typename T>
	struct __declspec(empty_bases)Decrementable
		: PreDecrementable<T>
		, PostDecrementable<T>
	{
		using PostDecrementable<T>::operator--;
		using PreDecrementable<T>::operator--;
	};

	template <typename T>
	struct __declspec(empty_bases)Arithmetic
		: Incrementable<T>
		, Decrementable<T>
		, Addable<T>
		, Subtractable<T>
		, Multiplicable<T>
		, Divisible<T>
		, Modulable<T>
		, BitWiseInvertable<T>
		, BitWiseAndable<T>
		, BitWiseOrable<T>
		, BitWiseXorable<T>
		, BitWiseLeftShiftable<T>
		, BitWiseRightShiftable<T>
		, Comparable<T>
		, Printable<T>
		, Hashable<T>
	{
	};

} // namespace fluent

namespace std
{
	export template <typename T, typename Parameter, template <typename> class... Skills>
	struct hash<Boring32::StrongTypes::NamedType<T, Parameter, Skills...>>
	{
		using NamedType = Boring32::StrongTypes::NamedType<T, Parameter, Skills...>;
		using checkIfHashable = typename std::enable_if<NamedType::is_hashable, void>::type;

		size_t operator()(Boring32::StrongTypes::NamedType<T, Parameter, Skills...> const& x) const noexcept
		{
			static_assert(noexcept(std::hash<T>()(x.get())), "hash fuction should not throw");

			return std::hash<T>()(x.get());
		}
	};

} // namespace std

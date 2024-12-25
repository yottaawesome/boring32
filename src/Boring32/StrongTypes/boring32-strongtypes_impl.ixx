export module boring32:strongtypes_impl;
import std;

export namespace Boring32::StrongTypes
{
	template <typename T>
	using IsNotReference = typename std::enable_if<!std::is_reference<T>::value, void>::type;

	template <typename T, typename Parameter, template <typename> class... Skills>
	class __declspec(empty_bases)NamedType : public Skills<NamedType<T, Parameter, Skills...>>...
	{
		public:
			using UnderlyingType = T;

			// constructor
			NamedType() = default;

			explicit constexpr NamedType(T const& value) noexcept(std::is_nothrow_copy_constructible<T>::value) : value_(value)
			{
			}

			template <typename T_ = T, typename = IsNotReference<T_>>
			explicit constexpr NamedType(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
				: value_(std::move(value))
			{
			}

			// get
			[[nodiscard]] constexpr T& get() noexcept
			{
				return value_;
			}

			[[nodiscard]] constexpr std::remove_reference_t<T> const& get() const noexcept
			{
				return value_;
			}

			// conversions
			using ref = NamedType<T&, Parameter, Skills...>;
			operator ref()
			{
				return ref(value_);
			}

			struct argument
			{
				NamedType operator=(T&& value) const
				{
					return NamedType(std::forward<T>(value));
				}
				template <typename U>
				NamedType operator=(U&& value) const
				{
					return NamedType(std::forward<U>(value));
				}

				argument() = default;
				argument(argument const&) = delete;
				argument(argument&&) = delete;
				argument& operator=(argument const&) = delete;
				argument& operator=(argument&&) = delete;
			};

		private:
			T value_;
	};

	template <template <typename T> class StrongType, typename T>
	constexpr StrongType<T> make_named(T const& value)
	{
		return StrongType<T>(value);
	}

	namespace details {
		template <class F, class... Ts>
		struct AnyOrderCallable {
			F f;
			template <class... Us>
			auto operator()(Us&&...args) const
			{
				static_assert(sizeof...(Ts) == sizeof...(Us), "Passing wrong number of arguments");
				auto x = std::make_tuple(std::forward<Us>(args)...);
				return f(std::move(std::get<Ts>(x))...);
			}
		};
	} //namespace details

	// EXPERIMENTAL - CAN BE CHANGED IN THE FUTURE. FEEDBACK WELCOME FOR IMPROVEMENTS!
	template <class... Args, class F>
	auto make_named_arg_function(F&& f)
	{
		return details::AnyOrderCallable<F, Args...>{std::forward<F>(f)};
	}
}

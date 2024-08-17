export module boring32:util_range;
import boring32.shared;
import :error;

namespace
{
	template<typename T>
	concept Rangeable = requires(T t1, T t2)
	{
		requires std::is_default_constructible_v<T>;
		t1++;
		t1 + t2;
		t1 += t2;
		t1 > t2;
		t1 >= t2;
		t1 < t2;
		t1 <= t2;
		t1 == t2;
	};

	template<typename T>
	concept NothrowIncrementable = requires(T t)
	{
		requires noexcept(t++);
	};

	template<typename T>
	concept NothrowAddable = requires(T t1, T t2)
	{
		requires noexcept(t1 + t2);
	};
}

export namespace Boring32::Util
{
	// A generic implementation of an inclusive range [m_min, m_max]
	// that supports any underlying type that can be incremented 
	// and compared.
	template<
		Rangeable TValue,
		bool VCopyConstructible = std::is_copy_constructible_v<TValue>,
		bool VCopyAssignable = std::is_copy_assignable_v<TValue>,
		bool VMoveConstructible = std::is_move_constructible_v<TValue>,
		bool VMoveAssignable = std::is_move_assignable_v<TValue>
	>
	class Range
	{
		public:
			enum class RangeError
			{
				Exhausted,
				InsufficientRemainder,
				IncrementFailure,
			};
		public:
			constexpr Range(const TValue min, const TValue max)
				: m_min(min), m_max(max)
			{
				if (m_min > m_max)
					throw Error::Boring32Error("Min greater than max");
			}

			Range(const Range& other) noexcept(std::is_nothrow_copy_constructible_v<TValue>)
				requires VCopyConstructible and std::is_copy_constructible_v<TValue> = default;

			Range& operator=(const Range& other) noexcept(std::is_nothrow_copy_assignable_v<TValue>)
				requires VCopyAssignable and std::is_copy_assignable_v<TValue> = default;

			Range(Range&& other) noexcept(std::is_nothrow_move_constructible_v<TValue>)
				requires VMoveConstructible and std::is_move_constructible_v<TValue> = default;

			Range& operator=(Range&& other) noexcept(std::is_nothrow_move_assignable_v<TValue>)
				requires VMoveAssignable and std::is_move_assignable_v<TValue> = default;

		public:
			TValue Next()
			{
				if (m_current > m_max)
					throw Error::Boring32Error("Range exceeded");
				return m_current++;
			}

			std::expected<TValue, RangeError> Next(const std::nothrow_t&) noexcept
			{
				if (m_current > m_max)
					return std::unexpected(RangeError::Exhausted);

				// Since we support any underlying type that supports ++, 
				// we have to check this.
				if constexpr (NothrowIncrementable<TValue>)
				{
					return m_current++;
				}
				else try
				{
					return m_current++;
				}
				catch (...)
				{
					return std::unexpected(RangeError::IncrementFailure);
				}
			}

			Range Next(const TValue next)
			{
				// Zero indexed. For example, imagine m_current = 0 and m_max = 1,
				// which gives us a range of 2 values. Now if we want all 2, we
				// get 0 + 2 > 1, which is wrong and would raise an exception, so 
				// we compare it against m_max + 1.
				if (m_current + next > m_max + 1)
					throw Error::Boring32Error("Insufficient remainder");

				auto temp = m_current;
				m_current += next;
				return { temp, m_current };
			}

			std::expected<TValue, RangeError> Next(const TValue next, const std::nothrow_t&) noexcept
			{
				if (m_current + next > m_max + 1)
					return std::unexpected(RangeError::InsufficientRemainder);

				if constexpr (NothrowAddable<TValue>)
				{
					return m_current += next;
				}
				else try
				{
					return m_current += next;
				}
				catch (...)
				{
					return std::unexpected(RangeError::IncrementFailure);
				}
			}

		private:
			TValue m_current{};
			TValue m_min{};
			TValue m_max{};
	};
}
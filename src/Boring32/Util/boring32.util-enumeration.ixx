export module boring32.util:enumeration;
import std;
import std.compat;
import boring32.error;

export namespace Boring32::Util
{
	template<typename T>
	concept HasSentinelMax = requires() { T::SentinelMax; };

	template<typename T>
	concept HasSentinelMin = requires() { T::SentinelMin; };

	template<typename T>
	concept HasSentinels = HasSentinelMin<T> and HasSentinelMax<T>;

	template<typename T>
	concept HasASentinel = HasSentinelMin<T> or HasSentinelMax<T>;

	template<typename T>
	concept HasNoSentinels = not HasASentinel<T>;
}

export namespace Boring32::Util
{
	template<typename T, T D = 0> requires std::is_enum_v<T>
	class Enum final
	{
		public:
			using UnderlyingType = std::underlying_type_t<T>;
			using EnumType = T;

		public:
			~Enum() = default;
			Enum() = default;
			Enum(const Enum&) = default;
			Enum& operator=(const Enum&) = default;
			Enum(Enum&&) noexcept = default;
			Enum& operator=(Enum&&) noexcept = default;

		public:
			Enum(UnderlyingType value)
				requires HasNoSentinels<T>
			{
				m_value = static_cast<T>(value);
			}

			Enum(UnderlyingType value)
				requires HasASentinel<T>
			{
				if (!IsValid(value))
					throw Error::Boring32Error("Value out of legal enum range");

				m_value = static_cast<T>(value);
			}

		public:
			operator UnderlyingType() const noexcept
			{
				return std::to_underlying<T>(m_value);
			}

			operator T() const noexcept
			{
				return m_value;
			}

			Enum& operator=(const UnderlyingType value) noexcept
				requires HasNoSentinels<T>
			{
				m_value = static_cast<T>(value);
			}

			Enum& operator=(const UnderlyingType value)
				requires HasSentinelMin<T> or HasSentinelMax<T>
			{
				if (!IsValid(value))
					throw Error::Boring32Error("Value out of legal enum range");

				m_value = static_cast<T>(value);
			}

			Enum& operator=(T other) noexcept
			{
				m_value = other;
			}

		public:
			bool IsValid(const UnderlyingType value) const noexcept
				requires HasSentinelMax<T> and not HasSentinelMin<T>
			{
				return IsBelowMax(value);
			}

			bool IsValid(const UnderlyingType value) const noexcept
				requires not HasSentinelMax<T> and HasSentinelMin<T>
			{
				return IsAboveMin(value);
			}

			bool IsValid(const UnderlyingType value) const noexcept
				requires HasSentinels<T>
			{
				return IsAboveMin(value) and IsBelowMax(value);
			}

			bool IsAboveMin(const UnderlyingType value) const noexcept
				requires HasSentinelMin<T>
			{
				return std::to_underlying(T::SentinelMin) < value;
			}

			bool IsBelowMax(const UnderlyingType value) const noexcept
				requires HasSentinelMax<T>
			{
				return value < std::to_underlying(T::SentinelMax);
			}

			T GetValue() const noexcept
			{
				return m_value;
			}

			UnderlyingType GetRawValue() const noexcept
			{
				return std::to_underlying(m_value);
			}

			T Default() const noexcept
			{
				return D;
			}

		private:
			T m_value = D;

			static_assert(
				not HasSentinelMin<T> or UnderlyingType(D) >= UnderlyingType(T::SentinelMin),
				"Default value must be >= SentinelMin."
			);
			static_assert(
				not HasSentinelMax<T> or UnderlyingType(D) <= UnderlyingType(T::SentinelMax),
				"Default value must be <= SentinelMax."
			);
			static_assert(
				HasNoSentinels<T> 
				or 
				HasSentinels<T> and UnderlyingType(T::SentinelMin) != (UnderlyingType(T::SentinelMax) - 1),
				"SentinelMin must be < SentinelMax and SentinelMax cannot immediately follow SentinelMin."
			);
	};

	/*enum class FailTest
	{
		SentinelMin,
		SentinelMax
	};
	using MM = Enum<FailTest, FailTest::SentinelMin>;
	MM m{};*/
}
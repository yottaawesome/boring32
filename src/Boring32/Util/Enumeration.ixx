export module boring32:util.enumeration;
import std;
import :error;

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
	template<typename T, T D = 0> 
	requires std::is_enum_v<T>
	struct Enum final
	{
		using UnderlyingType = std::underlying_type_t<T>;
		using EnumType = T;

		Enum(UnderlyingType value)
			requires HasNoSentinels<T>
		{
			m_value = static_cast<T>(value);
		}

		Enum(UnderlyingType value)
			requires HasASentinel<T>
		{
			if (not IsValid(value))
				throw Error::Boring32Error{ "Value out of legal enum range" };

			m_value = static_cast<T>(value);
		}

		operator UnderlyingType() const noexcept
		{
			return std::to_underlying<T>(m_value);
		}

		operator T() const noexcept
		{
			return m_value;
		}

		auto operator=(const UnderlyingType value) noexcept -> Enum&
			requires HasNoSentinels<T>
		{
			m_value = static_cast<T>(value);
			return *this;
		}

		auto operator=(const UnderlyingType value) -> Enum&
			requires (HasSentinelMin<T> or HasSentinelMax<T>)
		{
			if (not IsValid(value))
				throw Error::Boring32Error{ "Value out of legal enum range" };

			m_value = static_cast<T>(value);
			return *this;
		}

		auto operator=(T other) noexcept -> Enum&
		{
			m_value = other;
			return *this;
		}

		auto IsValid(const UnderlyingType value) const noexcept -> bool
			requires (HasSentinelMax<T> and not HasSentinelMin<T>)
		{
			return IsBelowMax(value);
		}

		auto IsValid(const UnderlyingType value) const noexcept -> bool
			requires (not HasSentinelMax<T> and HasSentinelMin<T>)
		{
			return IsAboveMin(value);
		}

		auto IsValid(const UnderlyingType value) const noexcept -> bool
			requires HasSentinels<T>
		{
			return IsAboveMin(value) and IsBelowMax(value);
		}

		auto IsAboveMin(const UnderlyingType value) const noexcept -> bool
			requires HasSentinelMin<T>
		{
			return std::to_underlying(T::SentinelMin) < value;
		}

		auto IsBelowMax(const UnderlyingType value) const noexcept -> bool
			requires HasSentinelMax<T>
		{
			return value < std::to_underlying(T::SentinelMax);
		}

		auto GetValue() const noexcept -> T
		{
			return m_value;
		}

		auto GetRawValue() const noexcept -> UnderlyingType
		{
			return std::to_underlying(m_value);
		}

		auto Default() const noexcept -> T
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
}
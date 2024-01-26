export module boring32.util:enumeration;
import std;
import std.compat;
import boring32.error;

export namespace Boring32::Util
{
	template<typename T>
	concept HasSentinelMax = requires(T a) { T::SentinelMax; };

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

			Enum& operator=(const UnderlyingType v) noexcept
				requires HasNoSentinels<T>
			{
				m_value = static_cast<T>(v);
				return *this;
			}

			Enum& operator=(const UnderlyingType v)
				requires HasSentinelMin<T> or HasSentinelMax<T>
			{
				if (!IsValid(v))
					throw Error::Boring32Error("Value out of legal enum range");

				m_value = static_cast<T>(v);
				return *this;
			}

			Enum& operator=(T t) noexcept
			{
				m_value = t;
				return *this;
			}

		public:
			bool IsValid(const UnderlyingType v) const noexcept
				requires HasSentinelMax<T> and not HasSentinelMin<T>
			{
				return IsBelowMax(v);
			}

			bool IsValid(const UnderlyingType v) const noexcept
				requires not HasSentinelMax<T> and HasSentinelMin<T>
			{
				return IsAboveMin(v);
			}

			bool IsValid(const UnderlyingType v) const noexcept
				requires HasSentinels<T>
			{
				return IsAboveMin(v) and IsBelowMax(v);
			}

			bool IsAboveMin(const UnderlyingType v) const noexcept
				requires HasSentinelMin<T>
			{
				return std::to_underlying(T::SentinelMin) < v;
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
	};
}
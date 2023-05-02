export module boring32.util:enumeration;
import <type_traits>;
import <utility>;
import boring32.error;

export namespace Boring32::Util
{
	template<typename T>
	concept HasSentinelMax = requires(T a)
	{
		T::SentinelMax;
	};

	template<typename T>
	concept HasSentinelMin = requires()
	{
		T::SentinelMin;
	};
}

export namespace Boring32::Util
{
	template<typename T> requires std::is_enum_v<T>
	class Enum final
	{
		public:
			~Enum() = default;
			Enum() = default;
			Enum(const Enum&) = default;
			Enum& operator=(const Enum&) = default;
			Enum(Enum&&) noexcept = default;
			Enum& operator=(Enum&&) noexcept = default;

		public:
			Enum(std::underlying_type_t<T> value)
				requires !HasSentinelMax<T> && !HasSentinelMin<T>
			{
				m_value = static_cast<T>(value);
			}

			Enum(std::underlying_type_t<T> value)
				requires HasSentinelMax<T> || HasSentinelMin<T>
			{
				if (!IsValid(value))
					throw Error::Boring32Error("Value out of legal enum range");

				m_value = static_cast<T>(value);
			}

		public:
			operator std::underlying_type_t<T>() const noexcept
			{
				return std::to_underlying<T>(m_value);
			}

			operator T() const noexcept
			{
				return m_value;
			}

			Enum& operator=(const std::underlying_type_t<T> v) noexcept 
				requires !HasSentinelMax<T> && !HasSentinelMin<T>
			{
				m_value = static_cast<T>(v);
				return *this;
			}

			Enum& operator=(const std::underlying_type_t<T> v) 
				requires HasSentinelMin<T> || HasSentinelMax<T>
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
			bool IsValid(const std::underlying_type_t<T> v) const noexcept
				requires HasSentinelMax<T> && !HasSentinelMin<T>
			{
				return IsBelowMax(v);
			}

			bool IsValid(const std::underlying_type_t<T> v) const noexcept
				requires !HasSentinelMax<T> && HasSentinelMin<T>
			{
				return IsAboveMin(v);
			}

			bool IsValid(const std::underlying_type_t<T> v) const noexcept
				requires HasSentinelMax<T> && HasSentinelMin<T>
			{
				return IsAboveMin(v) && IsBelowMax(v);
			}

			bool IsAboveMin(const std::underlying_type_t<T> v) const noexcept 
				requires HasSentinelMin<T>
			{
				return std::to_underlying(T::SentinelMin) < v;
			}

			bool IsBelowMax(const std::underlying_type_t<T> v) const noexcept
				requires HasSentinelMax<T>
			{
				return v < std::to_underlying(T::SentinelMax);
			}

			T GetValue() const noexcept
			{
				return m_value;
			}

			std::underlying_type_t<T> GetRawValue() const noexcept
			{
				return std::to_underlying(m_value);
			}

		private:
			T m_value{};
	};
}
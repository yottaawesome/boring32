module boring32:strongtypes_crtp;

namespace Boring32::StrongTypes
{
	template <typename T, template <typename> class crtpType>
	struct crtp
	{
		constexpr T& underlying()
		{
			return static_cast<T&>(*this);
		}
		constexpr T const& underlying() const
		{
			return static_cast<T const&>(*this);
		}
	};
}
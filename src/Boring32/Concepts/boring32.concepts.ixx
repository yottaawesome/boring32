export module boring32.concepts;
import std;
import std.compat;

export namespace Boring32::Concepts
{
	// Adapted from https://stackoverflow.com/a/41851068/7448661
	template<class T>
	struct IsDuration : std::false_type {};

	template<class Rep, class Period>
	struct IsDuration<std::chrono::duration<Rep, Period>> : std::true_type {};

	template<typename T>
	constexpr bool IsDurationV = IsDuration<T>::value;

	template<typename T>
	concept Duration = IsDuration<T>::value;

	template<typename T>
	concept NullPtrOrInvocable = std::is_null_pointer_v<T> or std::invocable<T>;

	template<typename T>
	struct IsPair : std::false_type {};

	template<typename T0, typename T1>
	struct IsPair<std::pair<T0, T1>> : std::true_type {};

	template<typename T>
	constexpr bool IsPairV = IsPair<T>::value;

	template<typename T>
	concept Pair = IsPairV<std::remove_cvref_t<T>>;

	template<typename T>
	struct IsArray : std::false_type {};

	template<typename T, size_t N>
	struct IsArray<std::array<T, N>> : std::true_type {};

	template<typename T, size_t N>
	constexpr bool IsArrayV = IsArray<T, N>;

	template<typename T>
	concept ArrayLike = IsArray<std::remove_cvref_t<T>>::value;

	template<typename T>
	struct IsVector : std::false_type {};

	template<typename T>
	struct IsVector<std::vector<T>> : std::true_type {};

	template<typename T>
	constexpr bool IsVectorV = IsVector<std::vector<T>>::value;

	template<typename T>
	concept VectorLike = IsArray<std::remove_cvref_t<T>>::value;
}
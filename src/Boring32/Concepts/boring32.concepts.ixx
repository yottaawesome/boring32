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

	template<typename T>
	concept WideOrNarrowString 
		= std::same_as<std::string, T> or std::same_as<std::wstring, T>;

	template<typename T>
	concept IsConvertibleToStringView = std::is_convertible_v<T, std::string_view>;
	
	template<typename T>
	concept IsConvertibleToWStringView = std::is_convertible_v<T, std::wstring_view>;

	// cvref is required
	// TODO: eliminate duplicate concepts
	template<typename T>
	concept IsString =
		std::is_same_v<std::remove_cvref_t<T>, std::string>
		or std::is_same_v<std::remove_cvref_t<T>, std::string_view>;

	template<typename T>
	concept IsWideString =
		std::is_same_v<std::remove_cvref_t<T>, std::wstring>
		or std::is_same_v<std::remove_cvref_t<T>, std::wstring_view>;

	template<typename T>
	concept IsStringType = IsString<T> or IsWideString<T>;

	template<typename T>
	concept AnyString = std::convertible_to<T, std::string_view> or std::convertible_to<T, std::wstring_view>;

	template <typename T>
	constexpr bool AlwaysFalse = std::false_type::value;
}
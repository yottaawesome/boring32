export module boring32.async:concepts;
import <type_traits>;
import <chrono>;

export namespace Boring32::Async
{
	// Adapted from https://stackoverflow.com/a/41851068/7448661
	template<class T>
	struct IsDuration : std::false_type {};

	template<class Rep, class Period>
	struct IsDuration<std::chrono::duration<Rep, Period>> : std::true_type {};

	template<typename T>
	concept Duration = IsDuration<T>::value;
}

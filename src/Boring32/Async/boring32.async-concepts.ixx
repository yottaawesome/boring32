// This should probably be converted into an internal module partition
export module boring32.async:concepts;
import <type_traits>;
import <chrono>;

namespace Boring32::Async
{
	template<class T>
	struct is_duration : std::false_type {};

	template<class Rep, class Period>
	struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

	template<typename T>
	concept Duration = is_duration<T>::value;
}

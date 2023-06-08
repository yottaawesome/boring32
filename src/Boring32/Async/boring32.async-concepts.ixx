export module boring32.async:concepts;
import <type_traits>;
import <chrono>;

namespace Boring32::Async
{
	template<typename T>
	concept IsDuration =
		std::is_same_v<T, std::chrono::milliseconds>
		|| std::is_same_v<T, std::chrono::seconds>
		|| std::is_same_v<T, std::chrono::minutes>
		|| std::is_same_v<T, std::chrono::hours>;
}

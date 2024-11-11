export module boring32:logging_functions;
import boring32.shared;
import :concepts;

export namespace Boring32::Log
{
	// https://www.cppstories.com/2021/non-terminal-variadic-args/
	template<typename...Ts>
	struct Logger
	{
		Logger(std::format_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current())
		{
			auto fmtString = std::format(fmt, std::forward<Ts>(args)...);
		}
		Logger(std::wformat_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current())
		{
			auto fmtString = std::format(fmt, std::forward<Ts>(args)...);
		}
	};
	template<typename... Ts>
	Logger(std::format_string<Ts...>, Ts&&...) -> Logger<Ts...>;
	template<typename... Ts>
	Logger(std::wformat_string<Ts...>, Ts&&...) -> Logger<Ts...>;

	template<typename... Ts>
	using Info = Logger<Ts...>;
	template<typename... Ts>
	using Warn = Logger<Ts...>;
	template<typename... Ts>
	using Error = Logger<Ts...>;
}

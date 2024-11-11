export module boring32:logging_functions;
import boring32.shared;
import :concepts;

export namespace Boring32::Log
{
	template<typename TString>
	struct FormatToString
	{
		TString Result;
	};
	using StringTarget = FormatToString<std::string>;
	using WStringTarget = FormatToString<std::wstring>;

	enum class Level
	{
		Debug,
		Info,
		Warn,
		Error
	};

	// https://www.cppstories.com/2021/non-terminal-variadic-args/
	template<Level, typename...Ts>
	struct Logger
	{
		constexpr Logger(StringTarget& out, std::format_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current()) noexcept
		{
			out.Result = std::format(fmt, std::forward<Ts>(args)...);
		}
		constexpr Logger(WStringTarget& out, std::wformat_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current()) noexcept
		{
			out.Result = std::format(fmt, std::forward<Ts>(args)...);
		}
		constexpr Logger(std::format_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current()) noexcept
		{
			auto fmtString = std::format(fmt, std::forward<Ts>(args)...);
		}
		constexpr Logger(std::wformat_string<Ts...> fmt, Ts&&...args, std::source_location loc = std::source_location::current()) noexcept
		{
			auto fmtString = std::format(fmt, std::forward<Ts>(args)...);
		}
	};
	template<Level VLevel, typename... Ts>
	Logger(std::format_string<Ts...>, Ts&&...) -> Logger<VLevel, Ts...>;
	template<Level VLevel, typename... Ts>
	Logger(std::wformat_string<Ts...>, Ts&&...) -> Logger<VLevel, Ts...>;
	template<Level VLevel, typename... Ts>
	Logger(StringTarget&, std::format_string<Ts...>, Ts&&...) -> Logger<VLevel, Ts...>;
	template<Level VLevel, typename... Ts>
	Logger(WStringTarget&, std::wformat_string<Ts...>, Ts&&...) -> Logger<VLevel, Ts...>;

	template<typename... Ts>
	using Info = Logger<Level::Info, Ts...>;
	template<typename... Ts>
	using Warn = Logger<Level::Warn, Ts...>;
	template<typename... Ts>
	using Error = Logger<Level::Error, Ts...>;

	void Test()
	{
		Info("{} {}", "a", 1);
	}
}

module;

#include <source_location>;

export module boring32.logging:functions;
import <string>;
import <format>;

export namespace Boring32::Logging
{
	template<typename T>
	concept IsConvertibleToStringView = std::is_convertible_v<T, std::string_view>;
	template<typename T>
	concept IsConvertibleToWStringView = std::is_convertible_v<T, std::wstring_view>;

	// Based on https://www.reddit.com/r/cpp_questions/comments/vl2n68/variadic_template_and_default_arguments/
	struct MessageAndLocation
	{
		std::string_view message;
		std::source_location loc;

		template<typename T>
		MessageAndLocation(
			T&& msg,
			std::source_location loc = std::source_location::current()
		) requires IsConvertibleToStringView<T>
			: message{ std::forward<T>(msg) },
			loc{ loc }
		{}
	};

	struct MessageAndLocationW
	{
		std::wstring_view message;
		std::source_location loc;

		template<typename T>
		MessageAndLocationW(
			T&& msg,
			std::source_location loc = std::source_location::current()
		) requires IsConvertibleToWStringView<T>
			: message{ std::forward<T>(msg) },
			loc{ loc }
		{}
	};

	template <class... Types>
	void Info(const std::string_view fmt, Types&&... Args)
	{
		std::vformat(fmt, std::make_format_args(std::forward<Types>(Args)...));
	}

	template <class... Types>
	void Info(const std::wstring_view fmt, Types&&... Args)
	{
	}

	template <class... Types>
	void Warn(const MessageAndLocation msgAndLoc, Types&&... Args)
	{
	}

	template <class... Types>
	void Warn(const MessageAndLocationW msgAndLoc, Types&&... Args)
	{
	}

	template <class... Types>
	void Warn(
		const MessageAndLocation msgAndLoc, 
		const std::exception& ex, 
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Warn(
		const MessageAndLocationW msgAndLoc,
		const std::exception& ex,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Exception(
		const MessageAndLocation msgAndLoc,
		const std::exception& ex,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Exception(
		const MessageAndLocationW msgAndLoc,
		const std::exception& ex,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocation msgAndLoc,
		const std::exception& ex,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocationW msgAndLoc,
		const std::exception& ex,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocation msgAndLoc,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Error(
		const MessageAndLocationW msgAndLoc,
		Types&&... Args
	)
	{
	}

	template <class... Types>
	void Debug(const std::string_view fmt, Types&&... Args)
	{
	}

	template <class... Types>
	void Debug(const std::wstring_view fmt, Types&&... Args)
	{
	}
}
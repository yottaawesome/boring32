module;

#include <source_location>;

export module boring32.error:boring32error;
import <stdexcept>;
import <string>;
import <string_view>;
import <format>;
import <source_location>;
import <stacktrace>;
import :functions;

export namespace Boring32::Error
{
	// Based on https://www.reddit.com/r/cpp_questions/comments/vl2n68/variadic_template_and_default_arguments/
	struct MessageLocationTrace
	{
		std::string_view Message;
		std::source_location Location;
		std::stacktrace Trace;

		template<typename T>
		MessageLocationTrace(
			T&& msg,
			std::source_location loc = std::source_location::current(),
			std::stacktrace trace = std::stacktrace::current()
		) : Message{ std::forward<T>(msg) },
			Location{ loc },
			Trace{ trace }
		{}
	};

	class Boring32Error : public std::exception
	{
		public:
			virtual ~Boring32Error() = default;
			Boring32Error()
				: m_message("Boring32 encountered an error")
			{ }
			Boring32Error(const Boring32Error& other) = default;
			virtual Boring32Error& operator=(const Boring32Error& other) = default;
			Boring32Error(Boring32Error&& other) noexcept = default;
			virtual Boring32Error& operator=(Boring32Error&& other) noexcept = default;

		public:
			Boring32Error(const MessageLocationTrace msg)
			{
				GenerateErrorMessage(
					msg.Location, 
					std::string(msg.Message), 
					msg.Trace
				);
			}

			Boring32Error(
				const std::string& message, 
				const std::source_location& location,
				const std::stacktrace& trace
			)
			{
				GenerateErrorMessage(location, message, trace);
			}

			template<typename...Args>
			Boring32Error(
				const std::string& message,
				const std::source_location& location,
				const std::stacktrace& trace,
				Args&&...args
			)
			{
				GenerateErrorMessage(
					location,
					std::vformat(
						message, 
						std::make_format_args(std::forward<Args>(args)...)
					),
					trace
				);
			}

		public:
			virtual const char* what() const noexcept override
			{
				return m_message.c_str();
			}

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message,
				const std::stacktrace& trace
			)
			{
				m_message = Error::FormatErrorMessage(
					"Boring32",
					trace,
					location, 
					message
				);
			}

		protected:
			std::string m_message;
	};
}
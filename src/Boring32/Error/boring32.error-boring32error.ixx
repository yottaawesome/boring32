export module boring32.error:boring32error;
import <stdexcept>;
import <string>;
import <format>;
import <source_location>;
import <stacktrace>;
import :functions;

export namespace Boring32::Error
{
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
			Boring32Error(
				const std::string& message, 
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			)
			{
				GenerateErrorMessage(location, message, trace);
			}

			template<typename...Args>
			Boring32Error(
				const std::string& message,
				const std::source_location& location,
				const std::stacktrace& trace,
				Args...args
			)
			{
				GenerateErrorMessage(
					location,
					std::vformat(
						message, 
						std::make_format_args(args...)
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
export module boring32.error:boring32error;
import <stdexcept>;
import <string>;
import <format>;
import <source_location>;

export namespace Boring32::Error
{
	class Boring32Error : public std::exception
	{
		public:
			virtual ~Boring32Error();
			Boring32Error(const Boring32Error& other);
			Boring32Error(Boring32Error&& other) noexcept;
			Boring32Error();
			Boring32Error(
				const std::string& message, 
				const std::source_location location = std::source_location::current()
			);
			template<typename...Args>
			Boring32Error(
				const std::string& message,
				const std::source_location location,
				Args...args
			)
			{
				GenerateErrorMessage(
					location,
					std::vformat(
						message, 
						std::make_format_args(args...)
					)
				);
			}

		public:
			virtual Boring32Error& operator=(const Boring32Error& other);
			virtual Boring32Error& operator=(Boring32Error&& other) noexcept;
			virtual const char* what() const noexcept override;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			);

		protected:
			std::string m_message;
	};
}
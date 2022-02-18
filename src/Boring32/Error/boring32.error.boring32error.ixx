module;

#include <stdexcept>
#include <string>
#include <source_location>

export module boring32.error.boring32error;

export namespace Boring32::Error
{
	class Boring32Error : public std::exception
	{
		public:
			virtual ~Boring32Error();
			Boring32Error(const std::source_location& location, const std::string& message);

		protected:
			virtual const char* what() const noexcept override;
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			);

		protected:
			std::string m_message;
	};
}
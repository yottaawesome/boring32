module;

#include <stdexcept>
#include <string>
#include <source_location>

export module boring32.error:boring32error;

export namespace Boring32::Error
{
	class Boring32Error : public std::exception
	{
		public:
			virtual ~Boring32Error();
			Boring32Error(const Boring32Error& other);
			Boring32Error(Boring32Error&& other) noexcept;
			Boring32Error(
				const std::string& message, 
				const std::source_location location = std::source_location::current()
			);

		protected:
			// Protected default constructor for base class use
			Boring32Error() = default;

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
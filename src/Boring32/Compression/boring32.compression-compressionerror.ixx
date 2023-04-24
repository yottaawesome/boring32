export module boring32.compression:compressionerror;
import boring32.error;
import <stdexcept>;
import <string>;
import <source_location>;

export namespace Boring32::Compression
{
	class CompressionError : public Error::Boring32Error
	{
		public:
			virtual ~CompressionError() = default;
			CompressionError(const CompressionError& other) = default;
			CompressionError(CompressionError&& other) noexcept = default;
			CompressionError(
				const std::string& message,
				const std::source_location location = std::source_location::current()
			) : Error::Boring32Error(message, location)
			{
				GenerateErrorMessage(location, message);
			}

		public:
			virtual CompressionError& operator=(const CompressionError& other) = default;
			virtual CompressionError& operator=(CompressionError&& other) noexcept = default;

		protected:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override
			{
				m_message = Error::FormatErrorMessage("Compression", location, message);
			}
	};
}

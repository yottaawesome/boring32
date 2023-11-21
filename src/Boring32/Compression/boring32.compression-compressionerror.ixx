export module boring32.compression:compressionerror;
import std;
import std.compat;
import boring32.error;

export namespace Boring32::Compression
{
	class CompressionError : public Error::Boring32Error
	{
		public:
			virtual ~CompressionError() = default;
			CompressionError() = default;
			CompressionError(const CompressionError& other) = default;
			virtual CompressionError& operator=(const CompressionError& other) = default;
			CompressionError(CompressionError&& other) noexcept = default;
			virtual CompressionError& operator=(CompressionError&& other) noexcept = default;

		public:
			CompressionError(
				const std::string& message,
				const std::source_location location = std::source_location::current(),
				const std::stacktrace& trace = std::stacktrace::current()
			) : Error::Boring32Error(message, location, trace)
			{
				GenerateErrorMessage(location, message, trace);
			}

		private:
			Error::ExactMessage GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message,
				const std::stacktrace& trace
			)
			{
				return { 
					Error::FormatErrorMessage(
						"Compression",
						trace,
						location,
						message
					)
				};
			}
	};
}

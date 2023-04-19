module boring32.compression:compressionerror;
import boring32.error;

namespace Boring32::Compression
{
	CompressionError::~CompressionError() = default;

	CompressionError::CompressionError(const CompressionError& other) = default;
	CompressionError::CompressionError(CompressionError&& other) noexcept = default;
	CompressionError& CompressionError::operator=(const CompressionError& other) = default;
	CompressionError& CompressionError::operator=(CompressionError&& other) noexcept = default;

	CompressionError::CompressionError(
		const std::string& message,
		const std::source_location location
	) : Error::Boring32Error(message, location)
	{
		GenerateErrorMessage(location, message);
	}

	void CompressionError::GenerateErrorMessage(
		const std::source_location & location,
		const std::string & message
	)
	{
		m_message = Error::FormatErrorMessage("Compression", location, message);
	}
}
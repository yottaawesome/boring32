module boring32.compression:compressionerror;
import boring32.error;

namespace Boring32::Compression
{
	CompressionError::CompressionError(
		const std::string& message,
		const std::source_location location
	) : Error::Boring32Error(message, location)
	{
		GenerateErrorMessage(location, message);
	}

	void CompressionError::GenerateErrorMessage(
		const std::source_location& location,
		const std::string& message
	)
	{
		m_message = Error::FormatErrorMessage("Compression", location, message);
	}
}
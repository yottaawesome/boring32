module boring32.error.compressionerror;
import boring32.error.functions;

namespace Boring32::Compression
{
	CompressionError::~CompressionError() = default;

	CompressionError::CompressionError(
		const std::source_location & location,
		const std::string & message
	) : Error::Boring32Error(location, message)
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
export module boring32.error.compressionerror;

import boring32.error.boring32error;
import <source_location>;
import <string>;

export namespace Boring32::Compression
{
	class CompressionError : public virtual Error::Boring32Error
	{
		public:
			virtual ~CompressionError();
			CompressionError(
				const std::source_location& location,
				const std::string& message
			);


		public:
			virtual void GenerateErrorMessage(
				const std::source_location& location,
				const std::string& message
			) override;

	};
}
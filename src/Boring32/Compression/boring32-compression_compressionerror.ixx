export module boring32:compression_compressionerror;
import boring32.shared;
import boring32.error;
import boring32.strings;

export namespace Boring32::Compression
{
	class CompressionError final : public Error::Boring32Error
	{
		public:
			CompressionError(
				const Error::MessageLocationTrace& msg, 
				auto&&...args
			) : Error::Boring32Error(CompressionError::GenerateErrorMessage(msg, std::forward<decltype(args)>(args)...))
			{ }

		private:
			static Error::ExactMessage GenerateErrorMessage(
				const Error::MessageLocationTrace& msg,
				auto&&...args
			)
			{
				return {
					Error::FormatErrorMessage(
						"Compression",
						msg.Trace,
						msg.Location,
						Strings::SafeVFormat(msg.Message, std::forward<decltype(args)>(args)...)
					)
				};
			}
	};
}

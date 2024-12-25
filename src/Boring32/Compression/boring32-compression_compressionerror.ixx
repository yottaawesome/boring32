export module boring32:compression_compressionerror;
import boring32.win32;
import :error;
import :strings;

export namespace Boring32::Compression
{
	struct CompressionError final : Error::Boring32Error
	{
		CompressionError(const Error::MessageLocationTrace& msg, auto&&...args) 
			: Error::Boring32Error(Generate(msg, std::forward<decltype(args)>(args)...))
		{ }

	private:
		static Boring32::Error::ExactMessage Generate(const Boring32::Error::MessageLocationTrace& msg, auto&&...args)
		{
			return {
				Boring32::Error::FormatErrorMessage(
					"Compression",
					msg.Trace,
					msg.Location,
					Boring32::Strings::SafeVFormat(msg.Message, std::forward<decltype(args)>(args)...)
				)
			};
		}
	};
}

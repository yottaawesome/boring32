export module boring32:compression_compressionerror;
import boring32.shared;
import :error;
import :strings;

namespace
{
	Boring32::Error::ExactMessage Generate(const Boring32::Error::MessageLocationTrace& msg, auto&&...args)
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
}

export namespace Boring32::Compression
{
	struct CompressionError final : public Error::Boring32Error
	{
		CompressionError(const Error::MessageLocationTrace& msg, auto&&...args) 
			: Error::Boring32Error(Generate(msg, std::forward<decltype(args)>(args)...))
		{ }
	};
}
